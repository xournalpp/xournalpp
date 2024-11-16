#include "SettingsDialog.h"

#include <algorithm>    // for max
#include <cstddef>      // for NULL, size_t
#include <type_traits>  // for __underlying_type_im...

#include <gdk/gdk.h>      // for GdkRGBA, GdkRectangle
#include <glib-object.h>  // for G_CALLBACK, g_signal...

#include "control/AudioController.h"             // for AudioController
#include "control/Control.h"                     // for Control
#include "control/DeviceListHelper.h"            // for getDeviceList, Input...
#include "control/settings/Settings.h"           // for Settings, SElement
#include "control/settings/SettingsEnums.h"      // for STYLUS_CURSOR_ARROW
#include "control/tools/StrokeStabilizerEnum.h"  // for AveragingMethod, Pre...
#include "gui/CreatePreviewImage.h"              // for createPreviewImage
#include "gui/MainWindow.h"                      // for MainWindow
#include "gui/XournalView.h"                     // for XournalView
#include "gui/toolbarMenubar/ToolMenuHandler.h"  // for ToolMenuHandler
#include "gui/toolbarMenubar/icon/ColorIcon.h"
#include "gui/toolbarMenubar/model/ColorPalette.h"  // for Palette
#include "gui/widgets/ZoomCallib.h"                 // for zoomcallib_new, zoom...
#include "model/PageType.h"                         // for PageType
#include "util/Color.h"                             // for GdkRGBA_to_argb, rgb...
#include "util/PathUtil.h"                          // for fromGFile
#include "util/Util.h"                              // for systemWithMessage
#include "util/gtk4_helper.h"                       //
#include "util/i18n.h"                              // for _
#include "util/raii/CairoWrappers.h"                // for CairoSurfaceSPtr
#include "util/safe_casts.h"                        // for round_cast

#include "ButtonConfigGui.h"       // for ButtonConfigGui
#include "DeviceClassConfigGui.h"  // for DeviceClassConfigGui
#include "LanguageConfigGui.h"     // for LanguageConfigGui
#include "LatexSettingsPanel.h"    // for LatexSettingsPanel
#include "filesystem.h"            // for is_directory

class GladeSearchpath;

using std::string;
using std::vector;

constexpr auto UI_FILE = "settings.glade";
constexpr auto UI_DIALOG_NAME = "settingsDialog";

SettingsDialog::SettingsDialog(GladeSearchpath* gladeSearchPath, Settings* settings, Control* control,
                               const std::vector<fs::path>& paletteDirectories, std::function<void()> callback):
        settings(settings),
        control(control),
        callib(zoomcallib_new()),
        builder(gladeSearchPath, UI_FILE),
        window(GTK_WINDOW(builder.get(UI_DIALOG_NAME))),
        languageConfig(gladeSearchPath, builder.get("hboxLanguageSelect"), settings),
        latexPanel(gladeSearchPath),
        paletteTab(gladeSearchPath, paletteDirectories),
        callback(callback) {

    gtk_box_append(GTK_BOX(builder.get("zoomVBox")), callib);
    gtk_widget_show(callib);

    GtkWidget* preview = xoj::helper::createPreviewImage(PageType{PageTypeFormat::Lined});
    gtk_box_append(GTK_BOX(builder.get("pagePreviewImage")), preview);
    gtk_widget_show(preview);

    initMouseButtonEvents(gladeSearchPath);

    vector<InputDevice> deviceList = DeviceListHelper::getDeviceList(this->settings);
    GtkBox* container = GTK_BOX(builder.get("hboxInputDeviceClasses"));
    for (const InputDevice& inputDevice: deviceList) {
        // Only add real devices (core pointers have vendor and product id nullptr)
        this->deviceClassConfigs.emplace_back(gladeSearchPath, container, settings, inputDevice);
    }
    if (deviceList.empty()) {
        GtkWidget* label = gtk_label_new("");
        gtk_label_set_markup(GTK_LABEL(label),
                             _("<b>No devices were found. This seems wrong - maybe file a bug report?</b>"));
        gtk_box_append(GTK_BOX(container), label);
        gtk_widget_show(label);
    }

    gtk_box_append(GTK_BOX(builder.get("latexTabBox")), this->latexPanel.getPanel());
    gtk_box_append(GTK_BOX(builder.get("paletteTabBox")), this->paletteTab.getPanel());

    g_signal_connect(GTK_RANGE(builder.get("zoomCallibSlider")), "value-changed",
                     G_CALLBACK(+[](GtkRange* range, gpointer self) {
                         static_cast<SettingsDialog*>(self)->setDpi(round_cast<int>(gtk_range_get_value(range)));
                     }),
                     this);

    g_signal_connect_swapped(builder.get("cbEnablePressureInference"), "toggled",
                             G_CALLBACK(+[](SettingsDialog* self) { self->updatePressureSensitivityOptions(); }), this);

    g_signal_connect_swapped(builder.get("cbSettingPresureSensitivity"), "toggled",
                             G_CALLBACK(+[](SettingsDialog* self) { self->updatePressureSensitivityOptions(); }), this);

    g_signal_connect_swapped(
            builder.get("cbAutosave"), "toggled",
            G_CALLBACK(+[](SettingsDialog* self) { self->enableWithCheckbox("cbAutosave", "boxAutosave"); }), this);

    g_signal_connect_swapped(builder.get("cbIgnoreFirstStylusEvents"), "toggled", G_CALLBACK(+[](SettingsDialog* self) {
                                 self->enableWithCheckbox("cbIgnoreFirstStylusEvents", "spNumIgnoredStylusEvents");
                             }),
                             this);


    g_signal_connect_swapped(builder.get("btTestEnable"), "clicked", G_CALLBACK(+[](SettingsDialog* self) {
                                 Util::systemWithMessage(gtk_editable_get_text(
                                         GTK_EDITABLE(self->builder.get("txtEnableTouchCommand"))));
                             }),
                             this);

    g_signal_connect_swapped(builder.get("btTestDisable"), "clicked", G_CALLBACK(+[](SettingsDialog* self) {
                                 Util::systemWithMessage(gtk_editable_get_text(
                                         GTK_EDITABLE(self->builder.get("txtDisableTouchCommand"))));
                             }),
                             this);

    g_signal_connect_swapped(builder.get("cbAddVerticalSpace"), "toggled", G_CALLBACK(+[](SettingsDialog* self) {
                                 self->enableWithCheckbox("cbAddVerticalSpace", "spAddVerticalSpaceAbove");
                                 self->enableWithCheckbox("cbAddVerticalSpace", "spAddVerticalSpaceBelow");
                             }),
                             this);

    g_signal_connect_swapped(builder.get("cbAddHorizontalSpace"), "toggled", G_CALLBACK(+[](SettingsDialog* self) {
                                 self->enableWithCheckbox("cbAddHorizontalSpace", "spAddHorizontalSpaceLeft");
                                 self->enableWithCheckbox("cbAddHorizontalSpace", "spAddHorizontalSpaceRight");
                             }),
                             this);

    g_signal_connect_swapped(builder.get("cbUnlimitedScrolling"), "toggled", G_CALLBACK(+[](SettingsDialog* self) {
                                 self->disableWithCheckbox("cbUnlimitedScrolling", "cbAddHorizontalSpace");
                                 self->disableWithCheckbox("cbUnlimitedScrolling", "cbAddVerticalSpace");
                                 self->enableWithEnabledCheckbox("cbAddHorizontalSpace", "spAddHorizontalSpaceLeft");
                                 self->enableWithEnabledCheckbox("cbAddVerticalSpace", "spAddVerticalSpaceAbove");
                                 self->enableWithEnabledCheckbox("cbAddHorizontalSpace", "spAddHorizontalSpaceRight");
                                 self->enableWithEnabledCheckbox("cbAddVerticalSpace", "spAddVerticalSpaceBelow");
                             }),
                             this);

    g_signal_connect_swapped(builder.get("cbDrawDirModsEnabled"), "toggled", G_CALLBACK(+[](SettingsDialog* self) {
                                 self->enableWithCheckbox("cbDrawDirModsEnabled", "spDrawDirModsRadius");
                             }),
                             this);

    g_signal_connect_swapped(builder.get("cbStrokeFilterEnabled"), "toggled", G_CALLBACK(+[](SettingsDialog* self) {
                                 self->enableWithCheckbox("cbStrokeFilterEnabled", "spStrokeIgnoreTime");
                                 self->enableWithCheckbox("cbStrokeFilterEnabled", "spStrokeIgnoreLength");
                                 self->enableWithCheckbox("cbStrokeFilterEnabled", "spStrokeSuccessiveTime");
                                 self->enableWithCheckbox("cbStrokeFilterEnabled", "cbDoActionOnStrokeFiltered");
                                 self->enableWithCheckbox("cbStrokeFilterEnabled", "cbTrySelectOnStrokeFiltered");
                             }),
                             this);

    g_signal_connect_swapped(builder.get("cbDisableAudio"), "toggled", G_CALLBACK(+[](SettingsDialog* self) {
                                 self->disableWithCheckbox("cbDisableAudio", "sidAudio1");
                                 self->disableWithCheckbox("cbDisableAudio", "sidAudio2");
                                 self->disableWithCheckbox("cbDisableAudio", "sidAudio3");
                                 self->disableWithCheckbox("cbDisableAudio", "sidAudio4");
                                 self->disableWithCheckbox("cbDisableAudio", "sidAudioLbl");
                             }),
                             this);

    g_signal_connect_swapped(builder.get("cbDisableTouchOnPenNear"), "toggled", G_CALLBACK(+[](SettingsDialog* self) {
                                 self->enableWithCheckbox("cbDisableTouchOnPenNear", "boxInternalHandRecognition");
                             }),
                             this);

    g_signal_connect_swapped(builder.get("cbTouchDisableMethod"), "changed",
                             G_CALLBACK(+[](SettingsDialog* self) { self->customHandRecognitionToggled(); }), this);

    g_signal_connect_swapped(builder.get("cbEnableZoomGestures"), "toggled", G_CALLBACK(+[](SettingsDialog* self) {
                                 self->enableWithCheckbox("cbEnableZoomGestures", "gdStartZoomAtSetting");
                             }),
                             this);

    g_signal_connect_swapped(builder.get("cbStylusCursorType"), "changed",
                             G_CALLBACK(+[](SettingsDialog* self) { self->customStylusIconTypeChanged(); }), this);

    g_signal_connect(GTK_COMBO_BOX(builder.get("cbStabilizerAveragingMethods")), "changed",
                     G_CALLBACK(+[](GtkComboBox* comboBox, gpointer self) {
                         static_cast<SettingsDialog*>(self)->showStabilizerAvMethodOptions(
                                 static_cast<StrokeStabilizer::AveragingMethod>(gtk_combo_box_get_active(comboBox)));
                     }),
                     this);

    g_signal_connect(GTK_COMBO_BOX(builder.get("cbStabilizerPreprocessors")), "changed",
                     G_CALLBACK(+[](GtkComboBox* comboBox, gpointer self) {
                         static_cast<SettingsDialog*>(self)->showStabilizerPreprocessorOptions(
                                 static_cast<StrokeStabilizer::Preprocessor>(gtk_combo_box_get_active(comboBox)));
                     }),
                     this);

    g_signal_connect_swapped(builder.get("btOk"), "clicked", G_CALLBACK(+[](SettingsDialog* self) {
                                 self->save();
                                 self->callback();
                                 gtk_window_close(self->window.get());
                             }),
                             this);
    g_signal_connect_swapped(builder.get("btCancel"), "clicked", G_CALLBACK(gtk_window_close), window.get());

    g_signal_connect(builder.get("cbUseSpacesAsTab"), "toggled",
                     G_CALLBACK(+[](GtkCheckButton* checkBox, SettingsDialog* self) {
                         self->enableWithCheckbox("cbUseSpacesAsTab", "numberOfSpacesContainer");
                     }),
                     this);

    load();
}

void SettingsDialog::initMouseButtonEvents(GladeSearchpath* gladeSearchPath) {
    auto emplaceButton = [gladeSearchPath, &btns = buttonConfigs, settings = settings, &bld = builder](
                                 const char* hbox, Button button, bool withDevice = false) {
        btns.emplace_back(std::make_unique<ButtonConfigGui>(gladeSearchPath, GTK_BOX(bld.get(hbox)), settings, button,
                                                            withDevice));
    };

    emplaceButton("hboxMiddleMouse", BUTTON_MOUSE_MIDDLE);
    emplaceButton("hboxRightMouse", BUTTON_MOUSE_RIGHT);
    emplaceButton("hboxEraser", BUTTON_ERASER);
    emplaceButton("hboxTouch", BUTTON_TOUCH, true);
    emplaceButton("hboxPenButton1", BUTTON_STYLUS_ONE);
    emplaceButton("hboxPenButton2", BUTTON_STYLUS_TWO);
    emplaceButton("hboxDefaultTool", BUTTON_DEFAULT);
}

void SettingsDialog::setDpi(int dpi) {
    if (this->dpi == dpi) {
        return;
    }

    this->dpi = dpi;
    zoomcallib_set_val(ZOOM_CALLIB(callib), dpi);
}

void SettingsDialog::loadCheckbox(const char* name, bool value) {
    gtk_check_button_set_active(GTK_CHECK_BUTTON(builder.get(name)), value);
}

auto SettingsDialog::getCheckbox(const char* name) -> bool {
    return gtk_check_button_get_active(GTK_CHECK_BUTTON(builder.get(name)));
}

void SettingsDialog::loadSlider(const char* name, double value) {
    GtkRange* range = GTK_RANGE(builder.get(name));
    gtk_range_set_value(range, value);
}

auto SettingsDialog::getSlider(const char* name) -> double {
    GtkRange* range = GTK_RANGE(builder.get(name));
    return gtk_range_get_value(range);
}

/**
 * Checkbox was toggled, enable / disable it
 */
void SettingsDialog::enableWithCheckbox(const string& checkboxId, const string& widgetId) {
    bool enabled = gtk_check_button_get_active(GTK_CHECK_BUTTON(builder.get(checkboxId)));
    gtk_widget_set_sensitive(builder.get(widgetId), enabled);
}

void SettingsDialog::disableWithCheckbox(const string& checkboxId, const string& widgetId) {
    bool enabled = gtk_check_button_get_active(GTK_CHECK_BUTTON(builder.get(checkboxId)));
    gtk_widget_set_sensitive(builder.get(widgetId), !enabled);
}

/**
 * similar to enableWithCheckbox, but also disables widget if the checkbox is disabled
 */
void SettingsDialog::enableWithEnabledCheckbox(const string& checkboxId, const string& widgetId) {
    GtkWidget* checkboxWidget = builder.get(checkboxId);
    bool const enabled =
            gtk_widget_get_sensitive(checkboxWidget) && gtk_check_button_get_active(GTK_CHECK_BUTTON(checkboxWidget));
    gtk_widget_set_sensitive(builder.get(widgetId), enabled);
}

void SettingsDialog::updatePressureSensitivityOptions() {
    GtkWidget* sensitivityOptionsFrame = builder.get("framePressureSensitivityScale");
    bool havePressureInput = getCheckbox("cbSettingPresureSensitivity") || getCheckbox("cbEnablePressureInference");

    if (!havePressureInput) {
        gtk_widget_set_tooltip_text(sensitivityOptionsFrame,
                                    _("Enable pressure sensitivity or pressure inference to change this setting!"));
    } else {

        gtk_widget_set_tooltip_text(sensitivityOptionsFrame,
                                    _("Filter input pressure. Multiply the pressure by the pressure multiplier."
                                      " If less than the minimum, use the minimum pressure."));
    }
    gtk_widget_set_sensitive(sensitivityOptionsFrame, havePressureInput);
}

void SettingsDialog::customHandRecognitionToggled() {
    GtkWidget* cbTouchDisableMethod = builder.get("cbTouchDisableMethod");
    int touchMethod = gtk_combo_box_get_active(GTK_COMBO_BOX(cbTouchDisableMethod));
    gtk_widget_set_sensitive(builder.get("boxCustomTouchDisableSettings"), touchMethod == 2);
}

void SettingsDialog::customStylusIconTypeChanged() {
    GtkWidget* cbStylusCursorType = builder.get("cbStylusCursorType");
    int stylusCursorType = gtk_combo_box_get_active(GTK_COMBO_BOX(cbStylusCursorType));
    bool showCursorHighlightOptions =
            (stylusCursorType != STYLUS_CURSOR_NONE && stylusCursorType != STYLUS_CURSOR_ARROW);
    gtk_widget_set_sensitive(builder.get("highlightCursorGrid"), showCursorHighlightOptions);
}

void SettingsDialog::showStabilizerAvMethodOptions(StrokeStabilizer::AveragingMethod method) {
    bool showArithmetic = method == StrokeStabilizer::AveragingMethod::ARITHMETIC;
    gtk_widget_set_visible(builder.get("lbStabilizerBuffersize"), showArithmetic);
    gtk_widget_set_visible(builder.get("sbStabilizerBuffersize"), showArithmetic);

    bool showSigma = method == StrokeStabilizer::AveragingMethod::VELOCITY_GAUSSIAN;
    gtk_widget_set_visible(builder.get("lbStabilizerSigma"), showSigma);
    gtk_widget_set_visible(builder.get("sbStabilizerSigma"), showSigma);

    bool preprocessorOn = static_cast<StrokeStabilizer::Preprocessor>(gtk_combo_box_get_active(GTK_COMBO_BOX(
                                  builder.get("cbStabilizerPreprocessors")))) != StrokeStabilizer::Preprocessor::NONE;
    bool sensitive = showSigma || showArithmetic || preprocessorOn;
    gtk_widget_set_sensitive(builder.get("cbStabilizerEnableFinalizeStroke"), sensitive);
}

void SettingsDialog::showStabilizerPreprocessorOptions(StrokeStabilizer::Preprocessor preprocessor) {
    bool showDeadzone = preprocessor == StrokeStabilizer::Preprocessor::DEADZONE;
    gtk_widget_set_visible(builder.get("lbStabilizerDeadzoneRadius"), showDeadzone);
    gtk_widget_set_visible(builder.get("sbStabilizerDeadzoneRadius"), showDeadzone);
    gtk_widget_set_visible(builder.get("cbStabilizerEnableCuspDetection"), showDeadzone);

    bool showInertia = preprocessor == StrokeStabilizer::Preprocessor::INERTIA;
    gtk_widget_set_visible(builder.get("lbStabilizerDrag"), showInertia);
    gtk_widget_set_visible(builder.get("sbStabilizerDrag"), showInertia);
    gtk_widget_set_visible(builder.get("lbStabilizerMass"), showInertia);
    gtk_widget_set_visible(builder.get("sbStabilizerMass"), showInertia);

    bool averagingOn = static_cast<StrokeStabilizer::AveragingMethod>(
                               gtk_combo_box_get_active(GTK_COMBO_BOX(builder.get("cbStabilizerAveragingMethods")))) !=
                       StrokeStabilizer::AveragingMethod::NONE;
    bool sensitive = showDeadzone || showInertia || averagingOn;
    gtk_widget_set_sensitive(builder.get("cbStabilizerEnableFinalizeStroke"), sensitive);
}

void SettingsDialog::load() {
    loadCheckbox("cbSettingPresureSensitivity", settings->isPressureSensitivity());
    loadCheckbox("cbEnableZoomGestures", settings->isZoomGesturesEnabled());
    loadCheckbox("cbShowSidebarRight", settings->isSidebarOnRight());
    loadCheckbox("cbShowScrollbarLeft", settings->isScrollbarOnLeft());
    loadCheckbox("cbAutoloadMostRecent", settings->isAutoloadMostRecent());
    loadCheckbox("cbAutoloadXoj", settings->isAutoloadPdfXoj());
    loadCheckbox("cbAutosave", settings->isAutosaveEnabled());
    loadCheckbox("cbAddVerticalSpace", settings->getAddVerticalSpace());
    loadCheckbox("cbAddHorizontalSpace", settings->getAddHorizontalSpace());
    loadCheckbox("cbUnlimitedScrolling", settings->getUnlimitedScrolling());
    loadCheckbox("cbDrawDirModsEnabled", settings->getDrawDirModsEnabled());
    loadCheckbox("cbStrokeFilterEnabled", settings->getStrokeFilterEnabled());
    loadCheckbox("cbDoActionOnStrokeFiltered", settings->getDoActionOnStrokeFiltered());
    loadCheckbox("cbTrySelectOnStrokeFiltered", settings->getTrySelectOnStrokeFiltered());
    loadCheckbox("cbSnapRecognizedShapesEnabled", settings->getSnapRecognizedShapesEnabled());
    loadCheckbox("cbRestoreLineWidthEnabled", settings->getRestoreLineWidthEnabled());
    loadCheckbox("cbStockIcons", settings->areStockIconsUsed());
    loadCheckbox("cbHideHorizontalScrollbar", settings->getScrollbarHideType() & SCROLLBAR_HIDE_HORIZONTAL);
    loadCheckbox("cbHideVerticalScrollbar", settings->getScrollbarHideType() & SCROLLBAR_HIDE_VERTICAL);
    loadCheckbox("cbDisableScrollbarFadeout", settings->isScrollbarFadeoutDisabled());
    loadCheckbox("cbDisableAudio", settings->isAudioDisabled());
    loadCheckbox("cbEnablePressureInference", settings->isPressureGuessingEnabled());
    loadCheckbox("cbTouchDrawing", settings->getTouchDrawingEnabled());
    loadCheckbox("cbDisableGtkInertialScroll", !settings->getGtkTouchInertialScrollingEnabled());
    const bool ignoreStylusEventsEnabled = settings->getIgnoredStylusEvents() != 0;  // 0 means disabled, >0 enabled
    loadCheckbox("cbIgnoreFirstStylusEvents", ignoreStylusEventsEnabled);
    loadCheckbox("cbInputSystemTPCButton", settings->getInputSystemTPCButtonEnabled());
    loadCheckbox("cbInputSystemDrawOutsideWindow", settings->getInputSystemDrawOutsideWindowEnabled());

    /**
     * Tab size related settings
     */
    loadCheckbox("cbUseSpacesAsTab", settings->getUseSpacesAsTab());
    GtkSpinButton* sbNumberOfSpacesForTab = GTK_SPIN_BUTTON(builder.get("sbNumberOfSpacesForTab"));
    gtk_spin_button_set_value(sbNumberOfSpacesForTab, settings->getNumberOfSpacesForTab());
    gtk_widget_set_sensitive(GTK_WIDGET(builder.get("numberOfSpacesContainer")), settings->getUseSpacesAsTab());

    /**
     * Stabilizer related settings
     */
    loadCheckbox("cbStabilizerEnableCuspDetection", settings->getStabilizerCuspDetection());
    loadCheckbox("cbStabilizerEnableFinalizeStroke", settings->getStabilizerFinalizeStroke());

    GtkWidget* sbStabilizerBuffersize = builder.get("sbStabilizerBuffersize");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(sbStabilizerBuffersize),
                              static_cast<double>(settings->getStabilizerBuffersize()));
    GtkWidget* sbStabilizerDeadzoneRadius = builder.get("sbStabilizerDeadzoneRadius");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(sbStabilizerDeadzoneRadius), settings->getStabilizerDeadzoneRadius());
    GtkWidget* sbStabilizerDrag = builder.get("sbStabilizerDrag");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(sbStabilizerDrag), settings->getStabilizerDrag());
    GtkWidget* sbStabilizerMass = builder.get("sbStabilizerMass");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(sbStabilizerMass), settings->getStabilizerMass());
    GtkWidget* sbStabilizerSigma = builder.get("sbStabilizerSigma");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(sbStabilizerSigma), settings->getStabilizerSigma());

    GtkComboBox* cbStabilizerAveragingMethods = GTK_COMBO_BOX(builder.get("cbStabilizerAveragingMethods"));
    gtk_combo_box_set_active(cbStabilizerAveragingMethods, static_cast<int>(settings->getStabilizerAveragingMethod()));
    showStabilizerAvMethodOptions(settings->getStabilizerAveragingMethod());
    GtkComboBox* cbStabilizerPreprocessors = GTK_COMBO_BOX(builder.get("cbStabilizerPreprocessors"));
    gtk_combo_box_set_active(cbStabilizerPreprocessors, static_cast<int>(settings->getStabilizerPreprocessor()));
    showStabilizerPreprocessorOptions(settings->getStabilizerPreprocessor());
    /***********/

    GtkComboBox* cbSidebarNumberingStyle = GTK_COMBO_BOX(builder.get("cbSidebarPageNumberStyle"));
    gtk_combo_box_set_active(cbSidebarNumberingStyle, static_cast<int>(settings->getSidebarNumberingStyle()));

    GtkWidget* txtDefaultSaveName = builder.get("txtDefaultSaveName");
    gtk_editable_set_text(GTK_EDITABLE(txtDefaultSaveName), settings->getDefaultSaveName().c_str());

    GtkWidget* txtDefaultPdfName = builder.get("txtDefaultPdfName");
    gtk_editable_set_text(GTK_EDITABLE(txtDefaultPdfName), settings->getDefaultPdfExportName().c_str());

    // gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(builder.get("fcAudioPath")),
    //                                     Util::toGFilename(settings->getAudioFolder()).c_str());

    GtkWidget* spAutosaveTimeout = builder.get("spAutosaveTimeout");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spAutosaveTimeout), settings->getAutosaveTimeout());

    GtkWidget* spNumIgnoredStylusEvents = builder.get("spNumIgnoredStylusEvents");
    if (!ignoreStylusEventsEnabled) {  // The spinButton's value should be >= 1
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(spNumIgnoredStylusEvents), 1);
    } else {
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(spNumIgnoredStylusEvents), settings->getIgnoredStylusEvents());
    }

    GtkWidget* spPairsOffset = builder.get("spPairsOffset");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spPairsOffset), settings->getPairsOffset());

    EmptyLastPageAppendType append = settings->getEmptyLastPageAppend();
    loadCheckbox("rdLastPageAppendOnDrawOfLastPage", append == EmptyLastPageAppendType::OnDrawOfLastPage);
    loadCheckbox("rdLastPageAppendOnScrollToEndOfLastPage", append == EmptyLastPageAppendType::OnScrollToEndOfLastPage);
    loadCheckbox("rdLastPageAppendDisabled", append == EmptyLastPageAppendType::Disabled);

    GtkWidget* spSnapRotationTolerance = builder.get("spSnapRotationTolerance");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spSnapRotationTolerance), settings->getSnapRotationTolerance());

    GtkWidget* spSnapGridTolerance = builder.get("spSnapGridTolerance");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spSnapGridTolerance), settings->getSnapGridTolerance());

    GtkWidget* spSnapGridSize = builder.get("spSnapGridSize");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spSnapGridSize), settings->getSnapGridSize() / DEFAULT_GRID_SIZE);

    GtkWidget* spStrokeRecognizerMinSize = builder.get("spStrokeRecognizerMinSize");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spStrokeRecognizerMinSize), settings->getStrokeRecognizerMinSize());

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(builder.get("edgePanSpeed")), settings->getEdgePanSpeed());
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(builder.get("edgePanMaxMult")), settings->getEdgePanMaxMult());

    GtkWidget* spZoomStep = builder.get("spZoomStep");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spZoomStep), settings->getZoomStep());

    GtkWidget* spZoomStepScroll = builder.get("spZoomStepScroll");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spZoomStepScroll), settings->getZoomStepScroll());

    GtkWidget* spAddHorizontalSpaceRight = builder.get("spAddHorizontalSpaceRight");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spAddHorizontalSpaceRight), settings->getAddHorizontalSpaceAmountRight());

    GtkWidget* spAddVerticalSpaceAbove = builder.get("spAddVerticalSpaceAbove");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spAddVerticalSpaceAbove), settings->getAddVerticalSpaceAmountAbove());

    GtkWidget* spAddHorizontalSpaceLeft = builder.get("spAddHorizontalSpaceLeft");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spAddHorizontalSpaceLeft), settings->getAddHorizontalSpaceAmountLeft());

    GtkWidget* spAddVerticalSpaceBelow = builder.get("spAddVerticalSpaceBelow");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spAddVerticalSpaceBelow), settings->getAddVerticalSpaceAmountBelow());

    GtkWidget* spDrawDirModsRadius = builder.get("spDrawDirModsRadius");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spDrawDirModsRadius), settings->getDrawDirModsRadius());

    GtkWidget* spReRenderThreshold = builder.get("spReRenderThreshold");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spReRenderThreshold), settings->getPDFPageRerenderThreshold());

    GtkWidget* spTouchZoomStartThreshold = builder.get("spTouchZoomStartThreshold");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spTouchZoomStartThreshold), settings->getTouchZoomStartThreshold());

    {
        int time = 0;
        double length = 0;
        int successive = 0;
        settings->getStrokeFilter(&time, &length, &successive);

        GtkWidget* spStrokeIgnoreTime = builder.get("spStrokeIgnoreTime");
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(spStrokeIgnoreTime), time);
        GtkWidget* spStrokeIgnoreLength = builder.get("spStrokeIgnoreLength");
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(spStrokeIgnoreLength), length);
        GtkWidget* spStrokeSuccessiveTime = builder.get("spStrokeSuccessiveTime");
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(spStrokeSuccessiveTime), successive);
    }

    this->setDpi(settings->getDisplayDpi());
    loadSlider("zoomCallibSlider", dpi);
    loadSlider("scaleMinimumPressure", settings->getMinimumPressure());
    loadSlider("scalePressureMultiplier", settings->getPressureMultiplier());

    GdkRGBA color = Util::rgb_to_GdkRGBA(settings->getBorderColor());
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(builder.get("colorBorder")), &color);
    color = Util::rgb_to_GdkRGBA(settings->getBackgroundColor());
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(builder.get("colorBackground")), &color);
    color = Util::rgb_to_GdkRGBA(settings->getSelectionColor());
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(builder.get("colorSelection")), &color);
    color = Util::rgb_to_GdkRGBA(settings->getActiveSelectionColor());
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(builder.get("colorSelectionActive")), &color);

    {
        const auto& recolor = settings->getRecolorParameters();
        loadCheckbox("cbRecolorDrawingArea", recolor.recolorizeMainView);
        loadCheckbox("cbRecolorPreviewSidebar", recolor.recolorizeSidebarMiniatures);
        color = Util::argb_to_GdkRGBA(recolor.recolor.getLight());
        gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(builder.get("recolorLight")), &color);
        color = Util::argb_to_GdkRGBA(recolor.recolor.getDark());
        gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(builder.get("recolorDark")), &color);
    }

    loadCheckbox("cbHighlightPosition", settings->isHighlightPosition());
    color = Util::argb_to_GdkRGBA(settings->getCursorHighlightColor());
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(builder.get("cursorHighlightColor")), &color);
    color = Util::argb_to_GdkRGBA(settings->getCursorHighlightBorderColor());
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(builder.get("cursorHighlightBorderColor")), &color);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(builder.get("cursorHighlightRadius")),
                              settings->getCursorHighlightRadius());
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(builder.get("cursorHighlightBorderWidth")),
                              settings->getCursorHighlightBorderWidth());

    switch (settings->getStylusCursorType()) {
        case STYLUS_CURSOR_NONE:
            gtk_combo_box_set_active(GTK_COMBO_BOX(builder.get("cbStylusCursorType")), 0);
            break;
        case STYLUS_CURSOR_BIG:
            gtk_combo_box_set_active(GTK_COMBO_BOX(builder.get("cbStylusCursorType")), 2);
            break;
        case STYLUS_CURSOR_ARROW:
            gtk_combo_box_set_active(GTK_COMBO_BOX(builder.get("cbStylusCursorType")), 3);
            break;
        case STYLUS_CURSOR_DOT:
        default:
            gtk_combo_box_set_active(GTK_COMBO_BOX(builder.get("cbStylusCursorType")), 1);
            break;
    }

    switch (settings->getEraserVisibility()) {
        case ERASER_VISIBILITY_NEVER:
            gtk_combo_box_set_active(GTK_COMBO_BOX(builder.get("cbEraserVisibility")), 0);
            break;
        case ERASER_VISIBILITY_HOVER:
            gtk_combo_box_set_active(GTK_COMBO_BOX(builder.get("cbEraserVisibility")), 2);
            break;
        case ERASER_VISIBILITY_TOUCH:
            gtk_combo_box_set_active(GTK_COMBO_BOX(builder.get("cbEraserVisibility")), 3);
            break;
        case ERASER_VISIBILITY_ALWAYS:
        default:
            gtk_combo_box_set_active(GTK_COMBO_BOX(builder.get("cbEraserVisibility")), 1);
            break;
    }

    switch (settings->getIconTheme()) {
        case ICON_THEME_LUCIDE:
            gtk_combo_box_set_active(GTK_COMBO_BOX(builder.get("cbIconTheme")), 1);
            break;
        case ICON_THEME_COLOR:
        default:
            gtk_combo_box_set_active(GTK_COMBO_BOX(builder.get("cbIconTheme")), 0);
            break;
    }

    switch (settings->getThemeVariant()) {
        case THEME_VARIANT_FORCE_LIGHT:
            gtk_combo_box_set_active(GTK_COMBO_BOX(builder.get("cbThemeVariant")), 1);
            break;
        case THEME_VARIANT_FORCE_DARK:
            gtk_combo_box_set_active(GTK_COMBO_BOX(builder.get("cbThemeVariant")), 2);
            break;
        case THEME_VARIANT_USE_SYSTEM:
        default:
            gtk_combo_box_set_active(GTK_COMBO_BOX(builder.get("cbThemeVariant")), 0);
            break;
    }

    auto viewMode = settings->getViewModes().at(PresetViewModeIds::VIEW_MODE_FULLSCREEN);
    bool showFullscreenMenubar = viewMode.showMenubar;
    bool showFullscreenToolbar = viewMode.showToolbar;
    bool showFullscreenSidebar = viewMode.showSidebar;
    viewMode = settings->getViewModes().at(PresetViewModeIds::VIEW_MODE_PRESENTATION);
    bool showPresentationMenubar = viewMode.showMenubar;
    bool showPresentationToolbar = viewMode.showToolbar;
    bool showPresentationSidebar = viewMode.showSidebar;
    bool goPresentationFullscreen = viewMode.goFullscreen;

    loadCheckbox("cbShowFullscreenMenubar", showFullscreenMenubar);
    loadCheckbox("cbShowFullscreenToolbar", showFullscreenToolbar);
    loadCheckbox("cbShowFullscreenSidebar", showFullscreenSidebar);
    loadCheckbox("cbShowPresentationMenubar", showPresentationMenubar);
    loadCheckbox("cbShowPresentationToolbar", showPresentationToolbar);
    loadCheckbox("cbShowPresentationSidebar", showPresentationSidebar);
    loadCheckbox("cbPresentationGoFullscreen", goPresentationFullscreen);
    loadCheckbox("cbHideMenubarStartup", settings->isMenubarVisible());
    loadCheckbox("cbShowFilepathInTitlebar", settings->isFilepathInTitlebarShown());
    loadCheckbox("cbShowPageNumberInTitlebar", settings->isPageNumberInTitlebarShown());

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(builder.get("preloadPagesBefore")),
                              static_cast<double>(settings->getPreloadPagesBefore()));
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(builder.get("preloadPagesAfter")),
                              static_cast<double>(settings->getPreloadPagesAfter()));
    loadCheckbox("cbEagerPageCleanup", settings->isEagerPageCleanup());

    disableWithCheckbox("cbUnlimitedScrolling", "cbAddVerticalSpace");
    disableWithCheckbox("cbUnlimitedScrolling", "cbAddHorizontalSpace");

    enableWithCheckbox("cbAutosave", "boxAutosave");
    enableWithCheckbox("cbIgnoreFirstStylusEvents", "spNumIgnoredStylusEvents");
    enableWithEnabledCheckbox("cbAddVerticalSpace", "spAddVerticalSpaceAbove");
    enableWithEnabledCheckbox("cbAddHorizontalSpace", "spAddHorizontalSpaceRight");
    enableWithEnabledCheckbox("cbAddVerticalSpace", "spAddVerticalSpaceBelow");
    enableWithEnabledCheckbox("cbAddHorizontalSpace", "spAddHorizontalSpaceLeft");
    enableWithCheckbox("cbDrawDirModsEnabled", "spDrawDirModsRadius");
    enableWithCheckbox("cbStrokeFilterEnabled", "spStrokeIgnoreTime");
    enableWithCheckbox("cbStrokeFilterEnabled", "spStrokeIgnoreLength");
    enableWithCheckbox("cbStrokeFilterEnabled", "spStrokeSuccessiveTime");
    enableWithCheckbox("cbStrokeFilterEnabled", "cbDoActionOnStrokeFiltered");
    enableWithCheckbox("cbStrokeFilterEnabled", "cbTrySelectOnStrokeFiltered");
    enableWithCheckbox("cbEnableZoomGestures", "gdStartZoomAtSetting");
    enableWithCheckbox("cbDisableTouchOnPenNear", "boxInternalHandRecognition");
    customHandRecognitionToggled();
    customStylusIconTypeChanged();
    updatePressureSensitivityOptions();


    SElement& touch = settings->getCustomElement("touch");
    bool disablePen = false;
    touch.getBool("disableTouch", disablePen);
    loadCheckbox("cbDisableTouchOnPenNear", disablePen);

    string disableMethod;
    touch.getString("method", disableMethod);
    int methodId = 0;
    if (disableMethod == "X11") {
        methodId = 1;
    } else if (disableMethod == "custom") {
        methodId = 2;
    }

    gtk_combo_box_set_active(GTK_COMBO_BOX(builder.get("cbTouchDisableMethod")), methodId);

    string cmd;
    touch.getString("cmdEnable", cmd);
    gtk_editable_set_text(GTK_EDITABLE(builder.get("txtEnableTouchCommand")), cmd.c_str());

    cmd = "";
    touch.getString("cmdDisable", cmd);
    gtk_editable_set_text(GTK_EDITABLE(builder.get("txtDisableTouchCommand")), cmd.c_str());

    int timeoutMs = 1000;
    touch.getInt("timeout", timeoutMs);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(builder.get("spTouchDisableTimeout")), timeoutMs / 1000.0);

    if (this->control->getAudioController()) {
        this->audioInputDevices = this->control->getAudioController()->getInputDevices();
        gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(builder.get("cbAudioInputDevice")), "", "System default");
        gtk_combo_box_set_active(GTK_COMBO_BOX(builder.get("cbAudioInputDevice")), 0);
        for (auto& audioInputDevice: this->audioInputDevices) {
            gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(builder.get("cbAudioInputDevice")), "",
                                      audioInputDevice.getDeviceName().c_str());
        }
        for (size_t i = 0; i < this->audioInputDevices.size(); i++) {
            if (this->audioInputDevices[i].getSelected()) {
                gtk_combo_box_set_active(GTK_COMBO_BOX(builder.get("cbAudioInputDevice")), static_cast<gint>(i + 1));
            }
        }

        this->audioOutputDevices = this->control->getAudioController()->getOutputDevices();
        gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(builder.get("cbAudioOutputDevice")), "", "System default");
        gtk_combo_box_set_active(GTK_COMBO_BOX(builder.get("cbAudioOutputDevice")), 0);
        for (auto& audioOutputDevice: this->audioOutputDevices) {
            gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(builder.get("cbAudioOutputDevice")), "",
                                      audioOutputDevice.getDeviceName().c_str());
        }
        for (size_t i = 0; i < this->audioOutputDevices.size(); i++) {
            if (this->audioOutputDevices[i].getSelected()) {
                gtk_combo_box_set_active(GTK_COMBO_BOX(builder.get("cbAudioOutputDevice")), static_cast<gint>(i + 1));
            }
        }

        switch (static_cast<int>(settings->getAudioSampleRate())) {
            case 16000:
                gtk_combo_box_set_active(GTK_COMBO_BOX(builder.get("cbAudioSampleRate")), 0);
                break;
            case 96000:
                gtk_combo_box_set_active(GTK_COMBO_BOX(builder.get("cbAudioSampleRate")), 2);
                break;
            case 192000:
                gtk_combo_box_set_active(GTK_COMBO_BOX(builder.get("cbAudioSampleRate")), 3);
                break;
            case 44100:
            default:
                gtk_combo_box_set_active(GTK_COMBO_BOX(builder.get("cbAudioSampleRate")), 1);
                break;
        }

        gtk_spin_button_set_value(GTK_SPIN_BUTTON(builder.get("spAudioGain")), settings->getAudioGain());
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(builder.get("spDefaultSeekTime")), settings->getDefaultSeekTime());
    }

    this->latexPanel.load(settings->latexSettings);
    paletteTab.renderPaletteTab(this->control->getPalette().getFilePath());
}

void SettingsDialog::save() {
    settings->transactionStart();

    settings->setPressureSensitivity(getCheckbox("cbSettingPresureSensitivity"));
    settings->setMinimumPressure(getSlider("scaleMinimumPressure"));
    settings->setPressureMultiplier(getSlider("scalePressureMultiplier"));
    settings->setZoomGesturesEnabled(getCheckbox("cbEnableZoomGestures"));
    settings->setSidebarOnRight(getCheckbox("cbShowSidebarRight"));
    settings->setScrollbarOnLeft(getCheckbox("cbShowScrollbarLeft"));
    settings->setAutoloadMostRecent(getCheckbox("cbAutoloadMostRecent"));
    settings->setAutoloadPdfXoj(getCheckbox("cbAutoloadXoj"));
    settings->setAutosaveEnabled(getCheckbox("cbAutosave"));
    settings->setAddVerticalSpace(getCheckbox("cbAddVerticalSpace"));
    settings->setAddHorizontalSpace(getCheckbox("cbAddHorizontalSpace"));
    settings->setUnlimitedScrolling(getCheckbox("cbUnlimitedScrolling"));
    settings->setDrawDirModsEnabled(getCheckbox("cbDrawDirModsEnabled"));
    settings->setStrokeFilterEnabled(getCheckbox("cbStrokeFilterEnabled"));
    settings->setDoActionOnStrokeFiltered(getCheckbox("cbDoActionOnStrokeFiltered"));
    settings->setTrySelectOnStrokeFiltered(getCheckbox("cbTrySelectOnStrokeFiltered"));
    settings->setSnapRecognizedShapesEnabled(getCheckbox("cbSnapRecognizedShapesEnabled"));
    settings->setRestoreLineWidthEnabled(getCheckbox("cbRestoreLineWidthEnabled"));
    settings->setAreStockIconsUsed(getCheckbox("cbStockIcons"));
    settings->setPressureGuessingEnabled(getCheckbox("cbEnablePressureInference"));
    settings->setTouchDrawingEnabled(getCheckbox("cbTouchDrawing"));
    settings->setGtkTouchInertialScrollingEnabled(!getCheckbox("cbDisableGtkInertialScroll"));
    settings->setInputSystemTPCButtonEnabled(getCheckbox("cbInputSystemTPCButton"));
    settings->setInputSystemDrawOutsideWindowEnabled(getCheckbox("cbInputSystemDrawOutsideWindow"));
    settings->setScrollbarFadeoutDisabled(getCheckbox("cbDisableScrollbarFadeout"));
    settings->setAudioDisabled(getCheckbox("cbDisableAudio"));

    /**
     * Tab size relate settings
     */
    settings->setUseSpacesAsTab(getCheckbox("cbUseSpacesAsTab"));
    settings->setNumberOfSpacesForTab(static_cast<unsigned int>(
            gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(builder.get("sbNumberOfSpacesForTab")))));

    /**
     * Stabilizer related settings
     */
    settings->setStabilizerAveragingMethod(static_cast<StrokeStabilizer::AveragingMethod>(
            gtk_combo_box_get_active(GTK_COMBO_BOX(builder.get("cbStabilizerAveragingMethods")))));
    settings->setStabilizerPreprocessor(static_cast<StrokeStabilizer::Preprocessor>(
            gtk_combo_box_get_active(GTK_COMBO_BOX(builder.get("cbStabilizerPreprocessors")))));
    settings->setStabilizerBuffersize(static_cast<size_t>(
            gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(builder.get("sbStabilizerBuffersize")))));
    settings->setStabilizerDeadzoneRadius(
            gtk_spin_button_get_value(GTK_SPIN_BUTTON(builder.get("sbStabilizerDeadzoneRadius"))));
    settings->setStabilizerDrag(gtk_spin_button_get_value(GTK_SPIN_BUTTON(builder.get("sbStabilizerDrag"))));
    settings->setStabilizerMass(gtk_spin_button_get_value(GTK_SPIN_BUTTON(builder.get("sbStabilizerMass"))));
    settings->setStabilizerSigma(gtk_spin_button_get_value(GTK_SPIN_BUTTON(builder.get("sbStabilizerSigma"))));
    settings->setStabilizerCuspDetection(getCheckbox("cbStabilizerEnableCuspDetection"));
    settings->setStabilizerFinalizeStroke(getCheckbox("cbStabilizerEnableFinalizeStroke"));

    settings->setSidebarNumberingStyle(static_cast<SidebarNumberingStyle>(
            gtk_combo_box_get_active(GTK_COMBO_BOX(builder.get("cbSidebarPageNumberStyle")))));

    auto scrollbarHideType =
            static_cast<std::make_unsigned<std::underlying_type<ScrollbarHideType>::type>::type>(SCROLLBAR_HIDE_NONE);
    if (getCheckbox("cbHideHorizontalScrollbar")) {
        scrollbarHideType |= SCROLLBAR_HIDE_HORIZONTAL;
    }
    if (getCheckbox("cbHideVerticalScrollbar")) {
        scrollbarHideType |= SCROLLBAR_HIDE_VERTICAL;
    }
    settings->setScrollbarHideType(static_cast<ScrollbarHideType>(scrollbarHideType));

    GdkRGBA color;
    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(builder.get("colorBorder")), &color);
    settings->setBorderColor(Util::GdkRGBA_to_argb(color));

    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(builder.get("colorBackground")), &color);
    settings->setBackgroundColor(Util::GdkRGBA_to_argb(color));

    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(builder.get("colorSelection")), &color);
    settings->setSelectionColor(Util::GdkRGBA_to_argb(color));

    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(builder.get("colorSelectionActive")), &color);
    settings->setActiveSelectionColor(Util::GdkRGBA_to_argb(color));

    {
        GdkRGBA color2;
        gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(builder.get("recolorLight")), &color);
        gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(builder.get("recolorDark")), &color2);

        settings->setRecolorParameters({getCheckbox("cbRecolorDrawingArea"), getCheckbox("cbRecolorPreviewSidebar"),
                                        Recolor(Util::GdkRGBA_to_argb(color), Util::GdkRGBA_to_argb(color2))});
    }

    settings->setHighlightPosition(getCheckbox("cbHighlightPosition"));
    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(builder.get("cursorHighlightColor")), &color);
    settings->setCursorHighlightColor(Util::GdkRGBA_to_argb(color));
    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(builder.get("cursorHighlightBorderColor")), &color);
    settings->setCursorHighlightBorderColor(Util::GdkRGBA_to_argb(color));
    GtkWidget* spCursorHighlightRadius = builder.get("cursorHighlightRadius");
    settings->setCursorHighlightRadius(gtk_spin_button_get_value(GTK_SPIN_BUTTON(spCursorHighlightRadius)));
    GtkWidget* spCursorHighlightBorderWidth = builder.get("cursorHighlightBorderWidth");
    settings->setCursorHighlightBorderWidth(gtk_spin_button_get_value(GTK_SPIN_BUTTON(spCursorHighlightBorderWidth)));

    switch (gtk_combo_box_get_active(GTK_COMBO_BOX(builder.get("cbStylusCursorType")))) {
        case 0:
            settings->setStylusCursorType(STYLUS_CURSOR_NONE);
            break;
        case 2:
            settings->setStylusCursorType(STYLUS_CURSOR_BIG);
            break;
        case 3:
            settings->setStylusCursorType(STYLUS_CURSOR_ARROW);
            break;
        case 1:
        default:
            settings->setStylusCursorType(STYLUS_CURSOR_DOT);
            break;
    }

    switch (gtk_combo_box_get_active(GTK_COMBO_BOX(builder.get("cbEraserVisibility")))) {
        case 0:
            settings->setEraserVisibility(ERASER_VISIBILITY_NEVER);
            break;
        case 2:
            settings->setEraserVisibility(ERASER_VISIBILITY_HOVER);
            break;
        case 3:
            settings->setEraserVisibility(ERASER_VISIBILITY_TOUCH);
            break;
        case 1:
        default:
            settings->setEraserVisibility(ERASER_VISIBILITY_ALWAYS);
            break;
    }

    switch (gtk_combo_box_get_active(GTK_COMBO_BOX(builder.get("cbIconTheme")))) {
        case 1:
            settings->setIconTheme(ICON_THEME_LUCIDE);
            break;
        case 0:
        default:
            settings->setIconTheme(ICON_THEME_COLOR);
            break;
    }

    switch (gtk_combo_box_get_active(GTK_COMBO_BOX(builder.get("cbThemeVariant")))) {
        case 1:
            settings->setThemeVariant(THEME_VARIANT_FORCE_LIGHT);
            break;
        case 2:
            settings->setThemeVariant(THEME_VARIANT_FORCE_DARK);
            break;
        case 0:
        default:
            settings->setThemeVariant(THEME_VARIANT_USE_SYSTEM);
            break;
    }

    ViewMode viewModeFullscreen;
    viewModeFullscreen.goFullscreen = true;
    viewModeFullscreen.showMenubar = getCheckbox("cbShowFullscreenMenubar");
    viewModeFullscreen.showToolbar = getCheckbox("cbShowFullscreenToolbar");
    viewModeFullscreen.showSidebar = getCheckbox("cbShowFullscreenSidebar");
    settings->setViewMode(PresetViewModeIds::VIEW_MODE_FULLSCREEN, viewModeFullscreen);

    ViewMode viewModePresentation;
    viewModePresentation.showMenubar = getCheckbox("cbShowPresentationMenubar");
    viewModePresentation.showToolbar = getCheckbox("cbShowPresentationToolbar");
    viewModePresentation.showSidebar = getCheckbox("cbShowPresentationSidebar");
    viewModePresentation.goFullscreen = getCheckbox("cbPresentationGoFullscreen");
    settings->setViewMode(PresetViewModeIds::VIEW_MODE_PRESENTATION, viewModePresentation);

    settings->setMenubarVisible(getCheckbox("cbHideMenubarStartup"));
    settings->setFilepathInTitlebarShown(getCheckbox("cbShowFilepathInTitlebar"));
    settings->setPageNumberInTitlebarShown(getCheckbox("cbShowPageNumberInTitlebar"));

    constexpr auto spinAsUint = [&](GtkSpinButton* btn) {
        int v = gtk_spin_button_get_value_as_int(btn);
        return v < 0 ? 0U : static_cast<unsigned int>(v);
    };
    unsigned int preloadPagesBefore = spinAsUint(GTK_SPIN_BUTTON(builder.get("preloadPagesBefore")));
    unsigned int preloadPagesAfter = spinAsUint(GTK_SPIN_BUTTON(builder.get("preloadPagesAfter")));
    settings->setPreloadPagesAfter(preloadPagesAfter);
    settings->setPreloadPagesBefore(preloadPagesBefore);
    settings->setEagerPageCleanup(getCheckbox("cbEagerPageCleanup"));

    settings->setDefaultSaveName(gtk_editable_get_text(GTK_EDITABLE(builder.get("txtDefaultSaveName"))));
    settings->setDefaultPdfExportName(gtk_editable_get_text(GTK_EDITABLE(builder.get("txtDefaultPdfName"))));
    // Todo(fabian): use Util::fromGFilename!
    auto file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(builder.get("fcAudioPath")));
    auto path = Util::fromGFile(file);
    g_object_unref(file);
    if (fs::is_directory(path)) {
        settings->setAudioFolder(path);
    }

    GtkWidget* spAutosaveTimeout = builder.get("spAutosaveTimeout");
    int autosaveTimeout = static_cast<int>(gtk_spin_button_get_value(GTK_SPIN_BUTTON(spAutosaveTimeout)));
    settings->setAutosaveTimeout(autosaveTimeout);

    if (getCheckbox("cbIgnoreFirstStylusEvents")) {
        GtkWidget* spNumIgnoredStylusEvents = builder.get("spNumIgnoredStylusEvents");
        int numIgnoredStylusEvents =
                static_cast<int>(gtk_spin_button_get_value(GTK_SPIN_BUTTON(spNumIgnoredStylusEvents)));
        settings->setIgnoredStylusEvents(numIgnoredStylusEvents);
    } else {
        settings->setIgnoredStylusEvents(0);  // This means nothing will be ignored
    }

    GtkWidget* spPairsOffset = builder.get("spPairsOffset");
    int numPairsOffset = static_cast<int>(gtk_spin_button_get_value(GTK_SPIN_BUTTON(spPairsOffset)));
    settings->setPairsOffset(numPairsOffset);

    if (getCheckbox("rdLastPageAppendDisabled")) {
        settings->setEmptyLastPageAppend(EmptyLastPageAppendType::Disabled);
    } else if (getCheckbox("rdLastPageAppendOnDrawOfLastPage")) {
        settings->setEmptyLastPageAppend(EmptyLastPageAppendType::OnDrawOfLastPage);
    } else if (getCheckbox("rdLastPageAppendOnScrollToEndOfLastPage")) {
        settings->setEmptyLastPageAppend(EmptyLastPageAppendType::OnScrollToEndOfLastPage);
    } else {
        settings->setEmptyLastPageAppend(EmptyLastPageAppendType::Disabled);
    }

    settings->setEdgePanSpeed(gtk_spin_button_get_value(GTK_SPIN_BUTTON(builder.get("edgePanSpeed"))));
    settings->setEdgePanMaxMult(gtk_spin_button_get_value(GTK_SPIN_BUTTON(builder.get("edgePanMaxMult"))));

    GtkWidget* spZoomStep = builder.get("spZoomStep");
    double zoomStep = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spZoomStep));
    settings->setZoomStep(zoomStep);

    GtkWidget* spZoomStepScroll = builder.get("spZoomStepScroll");
    double zoomStepScroll = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spZoomStepScroll));
    settings->setZoomStepScroll(zoomStepScroll);


    GtkWidget* spAddHorizontalSpaceRight = builder.get("spAddHorizontalSpaceRight");
    const int addHorizontalSpaceAmountRight =
            static_cast<int>(gtk_spin_button_get_value(GTK_SPIN_BUTTON(spAddHorizontalSpaceRight)));
    settings->setAddHorizontalSpaceAmountRight(addHorizontalSpaceAmountRight);

    GtkWidget* spAddVerticalSpaceAbove = builder.get("spAddVerticalSpaceAbove");
    const int addVerticalSpaceAmountAbove =
            static_cast<int>(gtk_spin_button_get_value(GTK_SPIN_BUTTON(spAddVerticalSpaceAbove)));
    settings->setAddVerticalSpaceAmountAbove(addVerticalSpaceAmountAbove);

    GtkWidget* spAddHorizontalSpaceLeft = builder.get("spAddHorizontalSpaceLeft");
    const int addHorizontalSpaceAmountLeft =
            static_cast<int>(gtk_spin_button_get_value(GTK_SPIN_BUTTON(spAddHorizontalSpaceLeft)));
    settings->setAddHorizontalSpaceAmountLeft(addHorizontalSpaceAmountLeft);

    GtkWidget* spAddVerticalSpaceBelow = builder.get("spAddVerticalSpaceBelow");
    const int addVerticalSpaceAmountBelow =
            static_cast<int>(gtk_spin_button_get_value(GTK_SPIN_BUTTON(spAddVerticalSpaceBelow)));
    settings->setAddVerticalSpaceAmountBelow(addVerticalSpaceAmountBelow);


    GtkWidget* spDrawDirModsRadius = builder.get("spDrawDirModsRadius");
    int drawDirModsRadius = static_cast<int>(gtk_spin_button_get_value(GTK_SPIN_BUTTON(spDrawDirModsRadius)));
    settings->setDrawDirModsRadius(drawDirModsRadius);

    GtkWidget* spStrokeIgnoreTime = builder.get("spStrokeIgnoreTime");
    int strokeIgnoreTime = static_cast<int>(gtk_spin_button_get_value(GTK_SPIN_BUTTON(spStrokeIgnoreTime)));
    GtkWidget* spStrokeIgnoreLength = builder.get("spStrokeIgnoreLength");
    double strokeIgnoreLength = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spStrokeIgnoreLength));
    GtkWidget* spStrokeSuccessiveTime = builder.get("spStrokeSuccessiveTime");
    int strokeSuccessiveTime = static_cast<int>(gtk_spin_button_get_value(GTK_SPIN_BUTTON(spStrokeSuccessiveTime)));
    settings->setStrokeFilter(strokeIgnoreTime, strokeIgnoreLength, strokeSuccessiveTime);

    GtkWidget* spTouchZoomStartThreshold = builder.get("spTouchZoomStartThreshold");
    double zoomStartThreshold = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spTouchZoomStartThreshold));
    settings->setTouchZoomStartThreshold(zoomStartThreshold);

    GtkWidget* spReRenderThreshold = builder.get("spReRenderThreshold");
    double rerenderThreshold = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spReRenderThreshold));
    settings->setPDFPageRerenderThreshold(rerenderThreshold);


    settings->setDisplayDpi(dpi);

    for (auto&& bcg: this->buttonConfigs) {
        bcg->saveSettings();
    }

    languageConfig.saveSettings();

    SElement& touch = settings->getCustomElement("touch");
    touch.setBool("disableTouch", getCheckbox("cbDisableTouchOnPenNear"));
    int touchMethod = gtk_combo_box_get_active(GTK_COMBO_BOX(builder.get("cbTouchDisableMethod")));

    switch (touchMethod) {
        case 1:
            touch.setString("method", "X11");
            break;
        case 2:
            touch.setString("method", "custom");
            break;
        case 0:
        default:
            touch.setString("method", "auto");
    }
    touch.setString("cmdEnable", gtk_editable_get_text(GTK_EDITABLE(builder.get("txtEnableTouchCommand"))));
    touch.setString("cmdDisable", gtk_editable_get_text(GTK_EDITABLE(builder.get("txtDisableTouchCommand"))));

    touch.setInt(
            "timeout",
            static_cast<int>(gtk_spin_button_get_value(GTK_SPIN_BUTTON(builder.get("spTouchDisableTimeout"))) * 1000));

    settings->setSnapRotationTolerance(
            static_cast<double>(gtk_spin_button_get_value(GTK_SPIN_BUTTON(builder.get("spSnapRotationTolerance")))));
    settings->setSnapGridTolerance(
            static_cast<double>(gtk_spin_button_get_value(GTK_SPIN_BUTTON(builder.get("spSnapGridTolerance")))));
    settings->setSnapGridSize(static_cast<double>(
            gtk_spin_button_get_value(GTK_SPIN_BUTTON(builder.get("spSnapGridSize"))) * DEFAULT_GRID_SIZE));

    settings->setStrokeRecognizerMinSize(
            static_cast<double>(gtk_spin_button_get_value(GTK_SPIN_BUTTON(builder.get("spStrokeRecognizerMinSize")))));

    size_t selectedInputDeviceIndex =
            static_cast<size_t>(gtk_combo_box_get_active(GTK_COMBO_BOX(builder.get("cbAudioInputDevice"))));
    if (selectedInputDeviceIndex == 0) {
        settings->setAudioInputDevice(Settings::AUDIO_INPUT_SYSTEM_DEFAULT);
    } else if (selectedInputDeviceIndex - 1 < this->audioInputDevices.size()) {
        settings->setAudioInputDevice(this->audioInputDevices[selectedInputDeviceIndex - 1].getIndex());
    }

    size_t selectedOutputDeviceIndex =
            static_cast<size_t>(gtk_combo_box_get_active(GTK_COMBO_BOX(builder.get("cbAudioOutputDevice"))));
    if (selectedOutputDeviceIndex == 0) {
        settings->setAudioOutputDevice(Settings::AUDIO_OUTPUT_SYSTEM_DEFAULT);
    } else if (selectedOutputDeviceIndex - 1 < this->audioOutputDevices.size()) {
        settings->setAudioOutputDevice(this->audioOutputDevices[selectedOutputDeviceIndex - 1].getIndex());
    }

    switch (gtk_combo_box_get_active(GTK_COMBO_BOX(builder.get("cbAudioSampleRate")))) {
        case 0:
            settings->setAudioSampleRate(16000.0);
            break;
        case 2:
            settings->setAudioSampleRate(96000.0);
            break;
        case 3:
            settings->setAudioSampleRate(192000.0);
            break;
        case 1:
        default:
            settings->setAudioSampleRate(44100.0);
            break;
    }

    settings->setAudioGain(static_cast<double>(gtk_spin_button_get_value(GTK_SPIN_BUTTON(builder.get("spAudioGain")))));
    settings->setDefaultSeekTime(
            static_cast<unsigned int>(gtk_spin_button_get_value(GTK_SPIN_BUTTON(builder.get("spDefaultSeekTime")))));

    const std::optional<std::filesystem::path> selectedPalette = paletteTab.getSelectedPalette();
    if (selectedPalette.has_value()) {
        settings->setColorPaletteSetting(selectedPalette.value());
    }

    for (auto& deviceClassConfigGui: this->deviceClassConfigs) {
        deviceClassConfigGui.saveSettings();
    }

    this->latexPanel.save(settings->latexSettings);

    settings->transactionEnd();

    this->control->getWindow()->setGtkTouchscreenScrollingForDeviceMapping();

    this->control->initButtonTool();
    this->control->getWindow()->getXournal()->onSettingsChanged();
}
