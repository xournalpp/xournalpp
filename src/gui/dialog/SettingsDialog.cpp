#include "SettingsDialog.h"

#include <utility>

#include <config.h>

#include "gui/widgets/ZoomCallib.h"

#include "ButtonConfigGui.h"
#include "DeviceListHelper.h"
#include "StringUtils.h"
#include "Util.h"
#include "i18n.h"

SettingsDialog::SettingsDialog(GladeSearchpath* gladeSearchPath, Settings* settings, Control* control):
        GladeGui(gladeSearchPath, "settings.glade", "settingsDialog"),
        settings(settings),
        control(control),
        callib(zoomcallib_new()),
        latexPanel(gladeSearchPath) {
    GtkWidget* vbox = get("zoomVBox");
    g_return_if_fail(vbox != nullptr);

    GtkWidget* slider = get("zoomCallibSlider");
    g_return_if_fail(slider != nullptr);

    g_signal_connect(slider, "change-value",
                     G_CALLBACK(+[](GtkRange* range, GtkScrollType scroll, gdouble value, SettingsDialog* self) {
                         self->setDpi((int)value);
                     }),
                     this);

    g_signal_connect(get("cbAutosave"), "toggled", G_CALLBACK(+[](GtkToggleButton* togglebutton, SettingsDialog* self) {
                         self->enableWithCheckbox("cbAutosave", "boxAutosave");
                     }),
                     this);

    g_signal_connect(get("cbIgnoreFirstStylusEvents"), "toggled",
                     G_CALLBACK(+[](GtkToggleButton* togglebutton, SettingsDialog* self) {
                         self->enableWithCheckbox("cbIgnoreFirstStylusEvents", "spNumIgnoredStylusEvents");
                     }),
                     this);


    g_signal_connect(get("btTestEnable"), "clicked", G_CALLBACK(+[](GtkButton* bt, SettingsDialog* self) {
                         Util::systemWithMessage(gtk_entry_get_text(GTK_ENTRY(self->get("txtEnableTouchCommand"))));
                     }),
                     this);

    g_signal_connect(get("btTestDisable"), "clicked", G_CALLBACK(+[](GtkButton* bt, SettingsDialog* self) {
                         Util::systemWithMessage(gtk_entry_get_text(GTK_ENTRY(self->get("txtDisableTouchCommand"))));
                     }),
                     this);

    g_signal_connect(get("cbAddVerticalSpace"), "toggled",
                     G_CALLBACK(+[](GtkToggleButton* togglebutton, SettingsDialog* self) {
                         self->enableWithCheckbox("cbAddVerticalSpace", "spAddVerticalSpace");
                     }),
                     this);

    g_signal_connect(get("cbAddHorizontalSpace"), "toggled",
                     G_CALLBACK(+[](GtkToggleButton* togglebutton, SettingsDialog* self) {
                         self->enableWithCheckbox("cbAddHorizontalSpace", "spAddHorizontalSpace");
                     }),
                     this);

    g_signal_connect(get("cbDrawDirModsEnabled"), "toggled",
                     G_CALLBACK(+[](GtkToggleButton* togglebutton, SettingsDialog* self) {
                         self->enableWithCheckbox("cbDrawDirModsEnabled", "spDrawDirModsRadius");
                     }),
                     this);

    g_signal_connect(get("cbStrokeFilterEnabled"), "toggled",
                     G_CALLBACK(+[](GtkToggleButton* togglebutton, SettingsDialog* self) {
                         self->enableWithCheckbox("cbStrokeFilterEnabled", "spStrokeIgnoreTime");
                         self->enableWithCheckbox("cbStrokeFilterEnabled", "spStrokeIgnoreLength");
                         self->enableWithCheckbox("cbStrokeFilterEnabled", "spStrokeSuccessiveTime");
                         self->enableWithCheckbox("cbStrokeFilterEnabled", "cbDoActionOnStrokeFiltered");
                         self->enableWithCheckbox("cbStrokeFilterEnabled", "cbTrySelectOnStrokeFiltered");
                     }),
                     this);

    g_signal_connect(get("cbDisableTouchOnPenNear"), "toggled",
                     G_CALLBACK(+[](GtkToggleButton* togglebutton, SettingsDialog* self) {
                         self->enableWithCheckbox("cbDisableTouchOnPenNear", "boxInternalHandRecognition");
                     }),
                     this);

    g_signal_connect(
            get("cbTouchDisableMethod"), "changed",
            G_CALLBACK(+[](GtkComboBox* comboBox, SettingsDialog* self) { self->customHandRecognitionToggled(); }),
            this);

    g_signal_connect(get("cbStylusCursorType"), "changed", G_CALLBACK(+[](GtkComboBox* comboBox, SettingsDialog* self) {
                         self->customStylusIconTypeChanged();
                     }),
                     this);

    gtk_box_pack_start(GTK_BOX(vbox), callib, false, true, 0);
    gtk_widget_show(callib);

    initLanguageSettings();
    initMouseButtonEvents();

    vector<InputDevice> deviceList = DeviceListHelper::getDeviceList(this->settings);
    GtkWidget* container = get("hboxInputDeviceClasses");
    for (const InputDevice& inputDevice: deviceList) {
        // Only add real devices (core pointers have vendor and product id nullptr)
        this->deviceClassConfigs.push_back(
                new DeviceClassConfigGui(getGladeSearchPath(), container, settings, inputDevice));
    }
    if (deviceList.empty()) {
        GtkWidget* label = gtk_label_new("");
        gtk_label_set_markup(GTK_LABEL(label),
                             _("<b>No devices were found. This seems wrong - maybe file a bug report?</b>"));
        gtk_box_pack_end(GTK_BOX(container), label, true, true, 0);
        gtk_widget_show(label);
    }

    gtk_container_add(GTK_CONTAINER(this->get("latexTabBox")), this->latexPanel.get("latexSettingsPanel"));
}

SettingsDialog::~SettingsDialog() {
    for (ButtonConfigGui* bcg: this->buttonConfigs) {
        delete bcg;
    }
    this->buttonConfigs.clear();

    for (DeviceClassConfigGui* dev: this->deviceClassConfigs) {
        delete dev;
    }
    this->deviceClassConfigs.clear();

    // DO NOT delete settings!
    this->settings = nullptr;
}

void SettingsDialog::initLanguageSettings() {
    languageConfig = std::make_unique<LanguageConfigGui>(getGladeSearchPath(), get("hboxLanguageSelect"), settings);
}

void SettingsDialog::initMouseButtonEvents(const char* hbox, int button, bool withDevice) {
    this->buttonConfigs.push_back(new ButtonConfigGui(getGladeSearchPath(), get(hbox), settings, button, withDevice));
}

void SettingsDialog::initMouseButtonEvents() {
    initMouseButtonEvents("hboxMidleMouse", BUTTON_MIDDLE);
    initMouseButtonEvents("hboxRightMouse", BUTTON_RIGHT);
    initMouseButtonEvents("hboxEraser", BUTTON_ERASER);
    initMouseButtonEvents("hboxTouch", BUTTON_TOUCH, true);
    initMouseButtonEvents("hboxPenButton1", BUTTON_STYLUS);
    initMouseButtonEvents("hboxPenButton2", BUTTON_STYLUS2);

    initMouseButtonEvents("hboxDefaultTool", BUTTON_DEFAULT);
}

void SettingsDialog::setDpi(int dpi) {
    if (this->dpi == dpi) {
        return;
    }

    this->dpi = dpi;
    zoomcallib_set_val(ZOOM_CALLIB(callib), dpi);
}

void SettingsDialog::show(GtkWindow* parent) {
    load();

    gtk_window_set_transient_for(GTK_WINDOW(this->window), parent);
    int res = gtk_dialog_run(GTK_DIALOG(this->window));

    if (res == 1) {
        this->save();
    }

    gtk_widget_hide(this->window);
}

void SettingsDialog::loadCheckbox(const char* name, gboolean value) {
    GtkToggleButton* b = GTK_TOGGLE_BUTTON(get(name));
    gtk_toggle_button_set_active(b, value);
}

auto SettingsDialog::getCheckbox(const char* name) -> bool {
    GtkToggleButton* b = GTK_TOGGLE_BUTTON(get(name));
    return gtk_toggle_button_get_active(b);
}

/**
 * Autosave was toggled, enable / disable autosave config
 */
void SettingsDialog::enableWithCheckbox(const string& checkbox, const string& widget) {
    GtkWidget* cbAutosave = get(checkbox);
    bool autosaveEnabled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cbAutosave));
    gtk_widget_set_sensitive(get(widget), autosaveEnabled);
}

void SettingsDialog::customHandRecognitionToggled() {
    GtkWidget* cbTouchDisableMethod = get("cbTouchDisableMethod");
    int touchMethod = gtk_combo_box_get_active(GTK_COMBO_BOX(cbTouchDisableMethod));
    gtk_widget_set_sensitive(get("boxCustomTouchDisableSettings"), touchMethod == 2);
}

void SettingsDialog::customStylusIconTypeChanged() {
    GtkWidget* cbStylusCursorType = get("cbStylusCursorType");
    int stylusCursorType = gtk_combo_box_get_active(GTK_COMBO_BOX(cbStylusCursorType));
    bool showCursorHighlightOptions = stylusCursorType != STYLUS_CURSOR_NONE;
    gtk_widget_set_sensitive(get("highlightCursorGrid"), showCursorHighlightOptions);
}

void SettingsDialog::load() {
    loadCheckbox("cbSettingPresureSensitivity", settings->isPressureSensitivity());
    loadCheckbox("cbEnableZoomGestures", settings->isZoomGesturesEnabled());
    loadCheckbox("cbShowSidebarRight", settings->isSidebarOnRight());
    loadCheckbox("cbShowScrollbarLeft", settings->isScrollbarOnLeft());
    loadCheckbox("cbAutoloadXoj", settings->isAutloadPdfXoj());
    loadCheckbox("cbAutosave", settings->isAutosaveEnabled());
    loadCheckbox("cbAddVerticalSpace", settings->getAddVerticalSpace());
    loadCheckbox("cbAddHorizontalSpace", settings->getAddHorizontalSpace());
    loadCheckbox("cbDrawDirModsEnabled", settings->getDrawDirModsEnabled());
    loadCheckbox("cbStrokeFilterEnabled", settings->getStrokeFilterEnabled());
    loadCheckbox("cbDoActionOnStrokeFiltered", settings->getDoActionOnStrokeFiltered());
    loadCheckbox("cbTrySelectOnStrokeFiltered", settings->getTrySelectOnStrokeFiltered());
    loadCheckbox("cbSnapRecognizedShapesEnabled", settings->getSnapRecognizedShapesEnabled());
    loadCheckbox("cbRestoreLineWidthEnabled", settings->getRestoreLineWidthEnabled());
    loadCheckbox("cbDarkTheme", settings->isDarkTheme());
    loadCheckbox("cbHideHorizontalScrollbar", settings->getScrollbarHideType() & SCROLLBAR_HIDE_HORIZONTAL);
    loadCheckbox("cbHideVerticalScrollbar", settings->getScrollbarHideType() & SCROLLBAR_HIDE_VERTICAL);
    loadCheckbox("cbDisableScrollbarFadeout", settings->isScrollbarFadeoutDisabled());
    loadCheckbox("cbTouchWorkaround", settings->isTouchWorkaround());
    const bool ignoreStylusEventsEnabled = settings->getIgnoredStylusEvents() != 0;  // 0 means disabled, >0 enabled
    loadCheckbox("cbIgnoreFirstStylusEvents", ignoreStylusEventsEnabled);
    loadCheckbox("cbNewInputSystem", settings->getExperimentalInputSystemEnabled());
    loadCheckbox("cbInputSystemTPCButton", settings->getInputSystemTPCButtonEnabled());
    loadCheckbox("cbInputSystemDrawOutsideWindow", settings->getInputSystemDrawOutsideWindowEnabled());


    GtkWidget* txtDefaultSaveName = get("txtDefaultSaveName");
    string txt = settings->getDefaultSaveName();
    gtk_entry_set_text(GTK_ENTRY(txtDefaultSaveName), txt.c_str());

    gtk_file_chooser_set_uri(GTK_FILE_CHOOSER(get("fcAudioPath")), settings->getAudioFolder().c_str());

    GtkWidget* spAutosaveTimeout = get("spAutosaveTimeout");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spAutosaveTimeout), settings->getAutosaveTimeout());

    GtkWidget* spNumIgnoredStylusEvents = get("spNumIgnoredStylusEvents");
    if (!ignoreStylusEventsEnabled) {  // The spinButton's value should be >= 1
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(spNumIgnoredStylusEvents), 1);
    } else {
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(spNumIgnoredStylusEvents), settings->getIgnoredStylusEvents());
    }

    GtkWidget* spPairsOffset = get("spPairsOffset");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spPairsOffset), settings->getPairsOffset());

    GtkWidget* spSnapRotationTolerance = get("spSnapRotationTolerance");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spSnapRotationTolerance), settings->getSnapRotationTolerance());

    GtkWidget* spSnapGridTolerance = get("spSnapGridTolerance");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spSnapGridTolerance), settings->getSnapGridTolerance());

    GtkWidget* spSnapGridSize = get("spSnapGridSize");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spSnapGridSize), settings->getSnapGridSize() / DEFAULT_GRID_SIZE);

    GtkWidget* spZoomStep = get("spZoomStep");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spZoomStep), settings->getZoomStep());

    GtkWidget* spZoomStepScroll = get("spZoomStepScroll");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spZoomStepScroll), settings->getZoomStepScroll());

    GtkWidget* spAddHorizontalSpace = get("spAddHorizontalSpace");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spAddHorizontalSpace), settings->getAddHorizontalSpaceAmount());

    GtkWidget* spAddVerticalSpace = get("spAddVerticalSpace");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spAddVerticalSpace), settings->getAddVerticalSpaceAmount());

    GtkWidget* spDrawDirModsRadius = get("spDrawDirModsRadius");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spDrawDirModsRadius), settings->getDrawDirModsRadius());

    {
        int time = 0;
        double length = 0;
        int successive = 0;
        settings->getStrokeFilter(&time, &length, &successive);

        GtkWidget* spStrokeIgnoreTime = get("spStrokeIgnoreTime");
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(spStrokeIgnoreTime), time);
        GtkWidget* spStrokeIgnoreLength = get("spStrokeIgnoreLength");
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(spStrokeIgnoreLength), length);
        GtkWidget* spStrokeSuccessiveTime = get("spStrokeSuccessiveTime");
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(spStrokeSuccessiveTime), successive);
    }

    GtkWidget* slider = get("zoomCallibSlider");

    this->setDpi(settings->getDisplayDpi());
    gtk_range_set_value(GTK_RANGE(slider), dpi);

    GdkRGBA color = Util::rgb_to_GdkRGBA(settings->getBorderColor());
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(get("colorBorder")), &color);
    color = Util::rgb_to_GdkRGBA(settings->getBackgroundColor());
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(get("colorBackground")), &color);
    color = Util::rgb_to_GdkRGBA(settings->getSelectionColor());
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(get("colorSelection")), &color);

    loadCheckbox("cbHighlightPosition", settings->isHighlightPosition());
    color = Util::argb_to_GdkRGBA(settings->getCursorHighlightColor());
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(get("cursorHighlightColor")), &color);
    color = Util::argb_to_GdkRGBA(settings->getCursorHighlightBorderColor());
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(get("cursorHighlightBorderColor")), &color);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(get("cursorHighlightRadius")), settings->getCursorHighlightRadius());
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(get("cursorHighlightBorderWidth")),
                              settings->getCursorHighlightBorderWidth());

    switch (settings->getStylusCursorType()) {
        case STYLUS_CURSOR_NONE:
            gtk_combo_box_set_active(GTK_COMBO_BOX(get("cbStylusCursorType")), 0);
            break;
        case STYLUS_CURSOR_BIG:
            gtk_combo_box_set_active(GTK_COMBO_BOX(get("cbStylusCursorType")), 2);
            break;
        case STYLUS_CURSOR_DOT:
        default:
            gtk_combo_box_set_active(GTK_COMBO_BOX(get("cbStylusCursorType")), 1);
            break;
    }

    bool hideFullscreenMenubar = false;
    bool hideFullscreenSidebar = false;
    bool hidePresentationMenubar = false;
    bool hidePresentationSidebar = false;

    string hidden = settings->getFullscreenHideElements();

    for (const string& element: StringUtils::split(hidden, ',')) {
        if (element == "mainMenubar") {
            hideFullscreenMenubar = true;
        } else if (element == "sidebarContents") {
            hideFullscreenSidebar = true;
        }
    }

    hidden = settings->getPresentationHideElements();
    for (const string& element: StringUtils::split(hidden, ',')) {
        if (element == "mainMenubar") {
            hidePresentationMenubar = true;
        } else if (element == "sidebarContents") {
            hidePresentationSidebar = true;
        }
    }

    loadCheckbox("cbHideFullscreenMenubar", hideFullscreenMenubar);
    loadCheckbox("cbHideFullscreenSidebar", hideFullscreenSidebar);
    loadCheckbox("cbHidePresentationMenubar", hidePresentationMenubar);
    loadCheckbox("cbHidePresentationSidebar", hidePresentationSidebar);
    loadCheckbox("cbHideMenubarStartup", settings->isMenubarVisible());

    enableWithCheckbox("cbAutosave", "boxAutosave");
    enableWithCheckbox("cbIgnoreFirstStylusEvents", "spNumIgnoredStylusEvents");
    enableWithCheckbox("cbAddVerticalSpace", "spAddVerticalSpace");
    enableWithCheckbox("cbAddHorizontalSpace", "spAddHorizontalSpace");
    enableWithCheckbox("cbDrawDirModsEnabled", "spDrawDirModsRadius");
    enableWithCheckbox("cbStrokeFilterEnabled", "spStrokeIgnoreTime");
    enableWithCheckbox("cbStrokeFilterEnabled", "spStrokeIgnoreLength");
    enableWithCheckbox("cbStrokeFilterEnabled", "spStrokeSuccessiveTime");
    enableWithCheckbox("cbStrokeFilterEnabled", "cbDoActionOnStrokeFiltered");
    enableWithCheckbox("cbStrokeFilterEnabled", "cbTrySelectOnStrokeFiltered");
    enableWithCheckbox("cbDisableTouchOnPenNear", "boxInternalHandRecognition");
    customHandRecognitionToggled();
    customStylusIconTypeChanged();


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

    gtk_combo_box_set_active(GTK_COMBO_BOX(get("cbTouchDisableMethod")), methodId);

    string cmd;
    touch.getString("cmdEnable", cmd);
    gtk_entry_set_text(GTK_ENTRY(get("txtEnableTouchCommand")), cmd.c_str());

    cmd = "";
    touch.getString("cmdDisable", cmd);
    gtk_entry_set_text(GTK_ENTRY(get("txtDisableTouchCommand")), cmd.c_str());

    int timeoutMs = 1000;
    touch.getInt("timeout", timeoutMs);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(get("spTouchDisableTimeout")), timeoutMs / 1000.0);

    this->audioInputDevices = this->control->getAudioController()->getInputDevices();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(get("cbAudioInputDevice")), "", "System default");
    gtk_combo_box_set_active(GTK_COMBO_BOX(get("cbAudioInputDevice")), 0);
    for (auto& audioInputDevice: this->audioInputDevices) {
        gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(get("cbAudioInputDevice")), "",
                                  audioInputDevice.getDeviceName().c_str());
    }
    for (size_t i = 0; i < this->audioInputDevices.size(); i++) {
        if (this->audioInputDevices[i].getSelected()) {
            gtk_combo_box_set_active(GTK_COMBO_BOX(get("cbAudioInputDevice")), i + 1);
        }
    }

    this->audioOutputDevices = this->control->getAudioController()->getOutputDevices();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(get("cbAudioOutputDevice")), "", "System default");
    gtk_combo_box_set_active(GTK_COMBO_BOX(get("cbAudioOutputDevice")), 0);
    for (auto& audioOutputDevice: this->audioOutputDevices) {
        gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(get("cbAudioOutputDevice")), "",
                                  audioOutputDevice.getDeviceName().c_str());
    }
    for (size_t i = 0; i < this->audioOutputDevices.size(); i++) {
        if (this->audioOutputDevices[i].getSelected()) {
            gtk_combo_box_set_active(GTK_COMBO_BOX(get("cbAudioOutputDevice")), i + 1);
        }
    }

    switch (static_cast<int>(settings->getAudioSampleRate())) {
        case 16000:
            gtk_combo_box_set_active(GTK_COMBO_BOX(get("cbAudioSampleRate")), 0);
            break;
        case 96000:
            gtk_combo_box_set_active(GTK_COMBO_BOX(get("cbAudioSampleRate")), 2);
            break;
        case 192000:
            gtk_combo_box_set_active(GTK_COMBO_BOX(get("cbAudioSampleRate")), 3);
            break;
        case 44100:
        default:
            gtk_combo_box_set_active(GTK_COMBO_BOX(get("cbAudioSampleRate")), 1);
            break;
    }

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(get("spAudioGain")), settings->getAudioGain());
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(get("spDefaultSeekTime")), settings->getDefaultSeekTime());

    this->latexPanel.load(settings->latexSettings);
}

auto SettingsDialog::updateHideString(const string& hidden, bool hideMenubar, bool hideSidebar) -> string {
    string newHidden;

    for (const string& element: StringUtils::split(hidden, ',')) {
        if (element == "mainMenubar") {
            if (hideMenubar) {
                hideMenubar = false;
            } else {
                continue;
            }
        } else if (element == "sidebarContents") {
            if (hideSidebar) {
                hideSidebar = false;
            } else {
                continue;
            }
        }

        if (!newHidden.empty()) {
            newHidden += ",";
        }
        newHidden += element;
    }

    if (hideMenubar) {
        if (!newHidden.empty()) {
            newHidden += ",";
        }
        newHidden += "mainMenubar";
    }

    if (hideSidebar) {
        if (!newHidden.empty()) {
            newHidden += ",";
        }
        newHidden += "sidebarContents";
    }

    return newHidden;
}

void SettingsDialog::save() {
    settings->transactionStart();

    settings->setPressureSensitivity(getCheckbox("cbSettingPresureSensitivity"));
    settings->setZoomGesturesEnabled(getCheckbox("cbEnableZoomGestures"));
    settings->setSidebarOnRight(getCheckbox("cbShowSidebarRight"));
    settings->setScrollbarOnLeft(getCheckbox("cbShowScrollbarLeft"));
    settings->setAutoloadPdfXoj(getCheckbox("cbAutoloadXoj"));
    settings->setAutosaveEnabled(getCheckbox("cbAutosave"));
    settings->setAddVerticalSpace(getCheckbox("cbAddVerticalSpace"));
    settings->setAddHorizontalSpace(getCheckbox("cbAddHorizontalSpace"));
    settings->setDrawDirModsEnabled(getCheckbox("cbDrawDirModsEnabled"));
    settings->setStrokeFilterEnabled(getCheckbox("cbStrokeFilterEnabled"));
    settings->setDoActionOnStrokeFiltered(getCheckbox("cbDoActionOnStrokeFiltered"));
    settings->setTrySelectOnStrokeFiltered(getCheckbox("cbTrySelectOnStrokeFiltered"));
    settings->setSnapRecognizedShapesEnabled(getCheckbox("cbSnapRecognizedShapesEnabled"));
    settings->setRestoreLineWidthEnabled(getCheckbox("cbRestoreLineWidthEnabled"));
    settings->setDarkTheme(getCheckbox("cbDarkTheme"));
    settings->setTouchWorkaround(getCheckbox("cbTouchWorkaround"));
    settings->setExperimentalInputSystemEnabled(getCheckbox("cbNewInputSystem"));
    settings->setInputSystemTPCButtonEnabled(getCheckbox("cbInputSystemTPCButton"));
    settings->setInputSystemDrawOutsideWindowEnabled(getCheckbox("cbInputSystemDrawOutsideWindow"));
    settings->setScrollbarFadeoutDisabled(getCheckbox("cbDisableScrollbarFadeout"));

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
    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(get("colorBorder")), &color);
    settings->setBorderColor(Util::GdkRGBA_to_argb(color));

    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(get("colorBackground")), &color);
    settings->setBackgroundColor(Util::GdkRGBA_to_argb(color));

    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(get("colorSelection")), &color);
    settings->setSelectionColor(Util::GdkRGBA_to_argb(color));


    settings->setHighlightPosition(getCheckbox("cbHighlightPosition"));
    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(get("cursorHighlightColor")), &color);
    settings->setCursorHighlightColor(Util::GdkRGBA_to_argb(color));
    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(get("cursorHighlightBorderColor")), &color);
    settings->setCursorHighlightBorderColor(Util::GdkRGBA_to_argb(color));
    GtkWidget* spCursorHighlightRadius = get("cursorHighlightRadius");
    settings->setCursorHighlightRadius(gtk_spin_button_get_value(GTK_SPIN_BUTTON(spCursorHighlightRadius)));
    GtkWidget* spCursorHighlightBorderWidth = get("cursorHighlightBorderWidth");
    settings->setCursorHighlightBorderWidth(gtk_spin_button_get_value(GTK_SPIN_BUTTON(spCursorHighlightBorderWidth)));

    switch (gtk_combo_box_get_active(GTK_COMBO_BOX(get("cbStylusCursorType")))) {
        case 0:
            settings->setStylusCursorType(STYLUS_CURSOR_NONE);
            break;
        case 2:
            settings->setStylusCursorType(STYLUS_CURSOR_BIG);
            break;
        case 1:
        default:
            settings->setStylusCursorType(STYLUS_CURSOR_DOT);
            break;
    }

    bool hideFullscreenMenubar = getCheckbox("cbHideFullscreenMenubar");
    bool hideFullscreenSidebar = getCheckbox("cbHideFullscreenSidebar");
    settings->setFullscreenHideElements(
            updateHideString(settings->getFullscreenHideElements(), hideFullscreenMenubar, hideFullscreenSidebar));

    bool hidePresentationMenubar = getCheckbox("cbHidePresentationMenubar");
    bool hidePresentationSidebar = getCheckbox("cbHidePresentationSidebar");
    settings->setPresentationHideElements(updateHideString(settings->getPresentationHideElements(),
                                                           hidePresentationMenubar, hidePresentationSidebar));

    settings->setMenubarVisible(getCheckbox("cbHideMenubarStartup"));

    settings->setDefaultSaveName(gtk_entry_get_text(GTK_ENTRY(get("txtDefaultSaveName"))));
    // Todo(fabian): use Util::fromGtkFilename!
    char* uri = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(get("fcAudioPath")));
    if (uri != nullptr) {
        settings->setAudioFolder(uri);
        g_free(uri);
    }

    GtkWidget* spAutosaveTimeout = get("spAutosaveTimeout");
    int autosaveTimeout = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spAutosaveTimeout));
    settings->setAutosaveTimeout(autosaveTimeout);

    if (getCheckbox("cbIgnoreFirstStylusEvents")) {
        GtkWidget* spNumIgnoredStylusEvents = get("spNumIgnoredStylusEvents");
        int numIgnoredStylusEvents = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spNumIgnoredStylusEvents));
        settings->setIgnoredStylusEvents(numIgnoredStylusEvents);
    } else {
        settings->setIgnoredStylusEvents(0);  // This means nothing will be ignored
    }

    GtkWidget* spPairsOffset = get("spPairsOffset");
    int numPairsOffset = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spPairsOffset));
    settings->setPairsOffset(numPairsOffset);

    GtkWidget* spZoomStep = get("spZoomStep");
    double zoomStep = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spZoomStep));
    settings->setZoomStep(zoomStep);

    GtkWidget* spZoomStepScroll = get("spZoomStepScroll");
    double zoomStepScroll = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spZoomStepScroll));
    settings->setZoomStepScroll(zoomStepScroll);


    GtkWidget* spAddHorizontalSpace = get("spAddHorizontalSpace");
    int addHorizontalSpaceAmount = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spAddHorizontalSpace));
    settings->setAddHorizontalSpaceAmount(addHorizontalSpaceAmount);

    GtkWidget* spAddVerticalSpace = get("spAddVerticalSpace");
    int addVerticalSpaceAmount = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spAddVerticalSpace));
    settings->setAddVerticalSpaceAmount(addVerticalSpaceAmount);

    GtkWidget* spDrawDirModsRadius = get("spDrawDirModsRadius");
    int drawDirModsRadius = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spDrawDirModsRadius));
    settings->setDrawDirModsRadius(drawDirModsRadius);

    GtkWidget* spStrokeIgnoreTime = get("spStrokeIgnoreTime");
    int strokeIgnoreTime = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spStrokeIgnoreTime));
    GtkWidget* spStrokeIgnoreLength = get("spStrokeIgnoreLength");
    double strokeIgnoreLength = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spStrokeIgnoreLength));
    GtkWidget* spStrokeSuccessiveTime = get("spStrokeSuccessiveTime");
    int strokeSuccessiveTime = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spStrokeSuccessiveTime));
    settings->setStrokeFilter(strokeIgnoreTime, strokeIgnoreLength, strokeSuccessiveTime);


    settings->setDisplayDpi(dpi);

    for (ButtonConfigGui* bcg: this->buttonConfigs) {
        bcg->saveSettings();
    }

    languageConfig->saveSettings();

    SElement& touch = settings->getCustomElement("touch");
    touch.setBool("disableTouch", getCheckbox("cbDisableTouchOnPenNear"));
    int touchMethod = gtk_combo_box_get_active(GTK_COMBO_BOX(get("cbTouchDisableMethod")));

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
    touch.setString("cmdEnable", gtk_entry_get_text(GTK_ENTRY(get("txtEnableTouchCommand"))));
    touch.setString("cmdDisable", gtk_entry_get_text(GTK_ENTRY(get("txtDisableTouchCommand"))));

    touch.setInt("timeout",
                 static_cast<int>(gtk_spin_button_get_value(GTK_SPIN_BUTTON(get("spTouchDisableTimeout"))) * 1000));

    settings->setSnapRotationTolerance(
            static_cast<double>(gtk_spin_button_get_value(GTK_SPIN_BUTTON(get("spSnapRotationTolerance")))));
    settings->setSnapGridTolerance(
            static_cast<double>(gtk_spin_button_get_value(GTK_SPIN_BUTTON(get("spSnapGridTolerance")))));
    settings->setSnapGridSize(
            static_cast<double>(gtk_spin_button_get_value(GTK_SPIN_BUTTON(get("spSnapGridSize"))) * DEFAULT_GRID_SIZE));

    int selectedInputDeviceIndex = gtk_combo_box_get_active(GTK_COMBO_BOX(get("cbAudioInputDevice"))) - 1;
    if (selectedInputDeviceIndex >= 0 && selectedInputDeviceIndex < static_cast<int>(this->audioInputDevices.size())) {
        settings->setAudioInputDevice(static_cast<int>(this->audioInputDevices[selectedInputDeviceIndex].getIndex()));
    }

    int selectedOutputDeviceIndex = gtk_combo_box_get_active(GTK_COMBO_BOX(get("cbAudioOutputDevice"))) - 1;
    if (selectedOutputDeviceIndex >= 0 &&
        selectedOutputDeviceIndex < static_cast<int>(this->audioOutputDevices.size())) {
        settings->setAudioOutputDevice(
                static_cast<int>(this->audioOutputDevices[selectedOutputDeviceIndex].getIndex()));
    }

    switch (gtk_combo_box_get_active(GTK_COMBO_BOX(get("cbAudioSampleRate")))) {
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

    settings->setAudioGain(static_cast<double>(gtk_spin_button_get_value(GTK_SPIN_BUTTON(get("spAudioGain")))));
    settings->setDefaultSeekTime(
            static_cast<double>(gtk_spin_button_get_value(GTK_SPIN_BUTTON(get("spDefaultSeekTime")))));

    for (DeviceClassConfigGui* deviceClassConfigGui: this->deviceClassConfigs) {
        deviceClassConfigGui->saveSettings();
    }

    this->latexPanel.save(settings->latexSettings);

    settings->transactionEnd();

    this->control->getWindow()->setTouchscreenScrollingForDeviceMapping();
}
