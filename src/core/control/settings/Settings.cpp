#include "Settings.h"

#include <algorithm>  // for max
#include <charconv>
#include <string_view>  // for literal sv
#include <type_traits>  // for add_const<>::type
#include <utility>      // for pair, move, make_...

#include <libxml/globals.h>    // for xmlFree, xmlInden...
#include <libxml/parser.h>     // for xmlKeepBlanksDefault
#include <libxml/xmlstring.h>  // for xmlChar

#include "control/DeviceListHelper.h"               // for InputDevice
#include "control/ToolEnums.h"                      // for ERASER_TYPE_NONE
#include "control/settings/LatexSettings.h"         // for LatexSettings
#include "control/settings/SettingsEnums.h"         // for InputDeviceTypeOp...
#include "gui/toolbarMenubar/model/ColorPalette.h"  // for Palette
#include "model/FormatDefinitions.h"                // for FormatUnits, XOJ_...
#include "util/Color.h"
#include "util/PathUtil.h"    // for getConfigFile
#include "util/Util.h"        // for PRECISION_FORMAT_...
#include "util/i18n.h"        // for _
#include "util/safe_casts.h"  // for as_unsigned
#include "util/utf8_view.h"   // for utf8_view

#include "ButtonConfig.h"  // for ButtonConfig
#include "config-dev.h"    // for PALETTE_FILE
#include "config-dev.h"
#include "filesystem.h"  // for path, exists
#include "xmlutils.h"    // for parse, cast, getXml

using std::string;
using namespace std::string_view_literals;

constexpr auto const* DEFAULT_FONT = "Sans";
constexpr auto DEFAULT_FONT_SIZE = 12;
constexpr auto DEFAULT_TOOLBAR = "Portrait";

#define SAVE_PROP(var) xmlNode = saveProperty(#var, var, root)

#define ATTACH_COMMENT(var)                     \
    com = xmlNewComment((const xmlChar*)(var)); \
    xmlAddPrevSibling(xmlNode, com);

Settings::~Settings() = default;

Settings::Settings(fs::path filepath): filepath(std::move(filepath)) {
    this->pressureSensitivity = true;
    this->minimumPressure = 0.05;
    this->pressureMultiplier = 1.0;
    this->pressureGuessing = false;
    this->zoomGesturesEnabled = true;

    this->maximized = false;
    this->showPairedPages = false;
    this->presentationMode = false;

    this->numColumns = 1;  // only one of these applies at a time
    this->numRows = 1;
    this->viewFixedRows = false;

    this->layoutVertical = false;
    this->layoutRightToLeft = false;
    this->layoutBottomToTop = false;

    this->numPairsOffset = 1;

    this->emptyLastPageAppend = EmptyLastPageAppendType::Disabled;

    this->edgePanSpeed = 20.0;
    this->edgePanMaxMult = 5.0;

    this->zoomStep = 10.0;
    this->zoomStepScroll = 2.0;

    this->displayDpi = -1;  // Automatic detection

    this->font.setName(DEFAULT_FONT);
    this->font.setSize(DEFAULT_FONT_SIZE);

    this->mainWndWidth = 800;
    this->mainWndHeight = 600;

    this->fullscreenActive = false;

    this->showSidebar = true;
    this->sidebarWidth = 150;
    this->sidebarNumberingStyle = SidebarNumberingStyle::DEFAULT;

    this->showToolbar = true;
    this->selectedToolbar = DEFAULT_TOOLBAR;

    this->sidebarOnRight = false;

    this->scrollbarOnLeft = false;

    this->menubarVisible = true;

    this->autoloadMostRecent = false;
    this->autoloadPdfXoj = true;

    this->stylusCursorType = STYLUS_CURSOR_DOT;
    this->eraserVisibility = ERASER_VISIBILITY_ALWAYS;
    this->iconTheme = ICON_THEME_COLOR;
    this->themeVariant = THEME_VARIANT_USE_SYSTEM;
    this->highlightPosition = false;
    this->cursorHighlightColor = 0x80FFFF00;  // Yellow with 50% opacity
    this->cursorHighlightRadius = 30.0;
    this->cursorHighlightBorderColor = 0x800000FF;  // Blue with 50% opacity
    this->cursorHighlightBorderWidth = 0.0;
    this->useStockIcons = false;
    this->scrollbarHideType = SCROLLBAR_HIDE_NONE;
    this->disableScrollbarFadeout = false;
    this->disableAudio = false;

    // Set this for autosave frequency in minutes.
    this->autosaveTimeout = 3;
    this->autosaveEnabled = true;

    this->addHorizontalSpace = false;
    this->addHorizontalSpaceAmountRight = 150;
    this->addHorizontalSpaceAmountLeft = 150;
    this->addVerticalSpace = false;
    this->addVerticalSpaceAmountAbove = 150;
    this->addVerticalSpaceAmountBelow = 150;

    this->unlimitedScrolling = false;

    // Drawing direction emulates modifier keys
    this->drawDirModsRadius = 50;
    this->drawDirModsEnabled = false;

    this->snapRotation = true;
    this->snapRotationTolerance = 0.30;

    this->snapGrid = true;
    this->snapGridTolerance = 0.50;
    this->snapGridSize = DEFAULT_GRID_SIZE;

    this->strokeRecognizerMinSize = 40;

    this->touchDrawing = false;
    this->gtkTouchInertialScrolling = true;

    this->defaultSaveName = xoj::util::utf8(_("%F-Note-%H-%M")).str();
    this->defaultPdfExportName = xoj::util::utf8(_("%{name}_annotated")).str();

    // Eraser
    this->buttonConfig[BUTTON_ERASER] = std::make_unique<ButtonConfig>(TOOL_ERASER, Colors::black, TOOL_SIZE_NONE,
                                                                       DRAWING_TYPE_DEFAULT, ERASER_TYPE_NONE);
    // Left button
    this->buttonConfig[BUTTON_MOUSE_LEFT] = std::make_unique<ButtonConfig>(TOOL_NONE, Colors::black, TOOL_SIZE_NONE,
                                                                           DRAWING_TYPE_DEFAULT, ERASER_TYPE_NONE);
    // Middle button
    this->buttonConfig[BUTTON_MOUSE_MIDDLE] = std::make_unique<ButtonConfig>(TOOL_HAND, Colors::black, TOOL_SIZE_NONE,
                                                                             DRAWING_TYPE_DEFAULT, ERASER_TYPE_NONE);
    // Right button
    this->buttonConfig[BUTTON_MOUSE_RIGHT] = std::make_unique<ButtonConfig>(TOOL_NONE, Colors::black, TOOL_SIZE_NONE,
                                                                            DRAWING_TYPE_DEFAULT, ERASER_TYPE_NONE);
    // Touch
    this->buttonConfig[BUTTON_TOUCH] = std::make_unique<ButtonConfig>(TOOL_NONE, Colors::black, TOOL_SIZE_NONE,
                                                                      DRAWING_TYPE_DEFAULT, ERASER_TYPE_NONE);
    // Default config
    this->buttonConfig[BUTTON_DEFAULT] = std::make_unique<ButtonConfig>(TOOL_PEN, Colors::black, TOOL_SIZE_FINE,
                                                                        DRAWING_TYPE_DEFAULT, ERASER_TYPE_NONE);
    // Pen button 1
    this->buttonConfig[BUTTON_STYLUS_ONE] = std::make_unique<ButtonConfig>(TOOL_NONE, Colors::black, TOOL_SIZE_NONE,
                                                                           DRAWING_TYPE_DEFAULT, ERASER_TYPE_NONE);
    // Pen button 2
    this->buttonConfig[BUTTON_STYLUS_TWO] = std::make_unique<ButtonConfig>(TOOL_NONE, Colors::black, TOOL_SIZE_NONE,
                                                                           DRAWING_TYPE_DEFAULT, ERASER_TYPE_NONE);

    // default view modes
    this->activeViewMode = PresetViewModeIds::VIEW_MODE_DEFAULT;
    this->viewModes =
            std::vector<ViewMode>{VIEW_MODE_STRUCT_DEFAULT, VIEW_MODE_STRUCT_FULLSCREEN, VIEW_MODE_STRUCT_PRESENTATION};

    this->touchZoomStartThreshold = 0.0;

    this->pageRerenderThreshold = 5.0;
    this->pdfPageCacheSize = 10;
    this->preloadPagesBefore = 3U;
    this->preloadPagesAfter = 5U;
    this->eagerPageCleanup = true;

    this->selectionBorderColor = Colors::red;
    this->selectionMarkerColor = Colors::xopp_cornflowerblue;
    this->activeSelectionColor = Colors::lawngreen;

    this->recolorParameters = {false, false, Recolor(Color{198, 208, 245}, Color{48, 52, 70})};

    this->backgroundColor = Colors::xopp_gainsboro02;

    // clang-format off
	this->pageTemplate = "xoj/template\ncopyLastPageSettings=true\nsize=595.275591x841.889764\nbackgroundType=lined\nbackgroundColor=#ffffff\n";
    // clang-format on

#ifdef ENABLE_AUDIO
    this->audioSampleRate = 44100.0;
    this->audioInputDevice = AUDIO_INPUT_SYSTEM_DEFAULT;
    this->audioOutputDevice = AUDIO_OUTPUT_SYSTEM_DEFAULT;
    this->audioGain = 1.0;
    this->defaultSeekTime = 5;
#endif

    this->pluginEnabled = "";
    this->pluginDisabled = "";

    this->numIgnoredStylusEvents = 0;

#ifdef _WIN32
    // This option should be on on Windows:
    // GTK (at least until 3.24.49) only creates GDK_BUTTON_PRESS events on mouse-events or stylus-down-events
    this->inputSystemTPCButton = true;
#else
    this->inputSystemTPCButton = false;
#endif

    this->inputSystemDrawOutsideWindow = true;

    this->strokeFilterIgnoreTime = 150;
    this->strokeFilterIgnoreLength = 1;
    this->strokeFilterSuccessiveTime = 500;
    this->strokeFilterEnabled = false;
    this->doActionOnStrokeFiltered = false;
    this->trySelectOnStrokeFiltered = false;

    this->snapRecognizedShapesEnabled = false;
    this->restoreLineWidthEnabled = false;

    this->inTransaction = false;

    /**
     * Stabilizer related settings
     */
    this->stabilizerAveragingMethod = StrokeStabilizer::AveragingMethod::NONE;
    this->stabilizerPreprocessor = StrokeStabilizer::Preprocessor::NONE;
    this->stabilizerBuffersize = 20;
    this->stabilizerSigma = 0.5;
    this->stabilizerDeadzoneRadius = 1.3;
    this->stabilizerCuspDetection = true;
    this->stabilizerDrag = 0.4;
    this->stabilizerMass = 5.0;
    this->stabilizerFinalizeStroke = true;
    /**/

    this->useSpacesForTab = false;
    this->numberOfSpacesForTab = 4;

    this->laserPointerFadeOutTime = 500;

    this->colorPaletteSetting = Util::getBuiltInPaletteDirectoryPath() / DEFAULT_PALETTE_FILE;
}

auto Settings::loadViewMode(ViewModeId mode) -> bool {
    if (mode < 0 || mode >= viewModes.size()) {
        return false;
    }
    auto viewMode = viewModes.at(mode);
    fullscreenActive = viewMode.goFullscreen;
    menubarVisible = viewMode.showMenubar;
    showToolbar = viewMode.showToolbar;
    showSidebar = viewMode.showSidebar;
    this->activeViewMode = mode;
    return true;
}

auto Settings::getViewModes() const -> const std::vector<ViewMode>& { return this->viewModes; }

auto Settings::getActiveViewMode() const -> ViewModeId { return this->activeViewMode; }

void Settings::parseData(xmlNodePtr cur, SElement& elem) {
    for (xmlNodePtr x = cur->children; x != nullptr; x = x->next) {
        const xmlChar* type = x->name;
        const auto name = xmlGet<string>(x, "name");

        if (type == "data"sv) {
            parseData(x, elem.child(name));
        } else if (type == "attribute"sv) {
            switch (xmlGet<AttributeType>(x, "type")) {
                case ATTRIBUTE_TYPE_INT: {
                    const auto value = xmlGet<int>(x, "value");
                    elem.set(name, value);
                    break;
                }
                case ATTRIBUTE_TYPE_DOUBLE: {
                    const auto value = xmlGet<double>(x, "value");
                    elem.set(name, value);
                    break;
                }
                case ATTRIBUTE_TYPE_INT_HEX: {
                    const auto value = xmlGet<string>(x, "value");
                    if (const auto i = static_cast<uint32_t>(std::stoull(value, nullptr, 16))) {
                        elem.set(name, i);
                    } else {
                        g_warning("Settings::Unknown hex value: %s:%s\n", name.c_str(), value.c_str());
                    }
                    break;
                }
                case ATTRIBUTE_TYPE_STRING: {
                    const auto value = xmlGet<string>(x, "value");
                    elem.set(name, value);
                    break;
                }
                case ATTRIBUTE_TYPE_BOOLEAN: {
                    const auto value = xmlGet<bool>(x, "value");
                    elem.set(name, value);
                    break;
                }
                default: {
                    const auto sType = xmlGet<string>(x, "type");
                    g_warning("Settings::Unknown datatype: %s\n", sType.c_str());
                    break;
                }
            }
        } else {
            g_warning("Settings::parseData: Unknown XML node: %s\n", x->name);
        }
    }
}

void Settings::parseItem(xmlDocPtr doc, xmlNodePtr cur) {
    const xmlChar* nameID = cur->name;
    // Parse data map
    if (nameID == "data"sv) {
        const auto name = xmlGet<string>(cur, "name");
        if (name.empty()) {
            g_warning("Settings::%s:No name property!\n", nameID);
            return;
        }

        parseData(cur, data[name]);

        return;
    }

    if (cur->type == XML_COMMENT_NODE) {
        return;
    }

    if (cur->name != "property"sv) {
        g_warning("Settings::Unknown XML node: %s\n", cur->name);
        return;
    }

    const auto name = xmlGet<string>(cur, "name");
    if (name.empty()) {
        g_warning("Settings::%s:No name property!\n", cur->name);
        return;
    }

    const auto value = xmlGet<std::string_view>(cur, "value");

    const auto fontname = xmlGet<std::string>(cur, "font");
    const auto size = xmlGet<double>(cur, "size", DEFAULT_FONT_SIZE);
    const auto newFont = XojFont(fontname, size);

    if (value.empty() && fontname.empty()) {
        g_warning("Settings::No %s property: %s", value.empty() ? "Value" : "Font", name.c_str());
        return;
    }

    // TODO(fabian): remove this typo fix in 2-3 release cycles
    if (name == "presureSensitivity") {
        setParsed(this->pressureSensitivity, value);
    }
    if (name == "pressureSensitivity") {
        setParsed(this->pressureSensitivity, value);
    } else if (name == "font") {
        this->font = newFont;
    } else if (name == "minimumPressure") {
        // std::max is for backwards compatibility for users who might have set this value too small
        this->minimumPressure = std::max(0.01, parse<double>(value));
    } else if (name == "pressureMultiplier") {
        setParsed(this->pressureMultiplier, value);
    } else if (name == "zoomGesturesEnabled") {
        setParsed(this->zoomGesturesEnabled, value);
    } else if (name == "selectedToolbar") {
        setParsed(this->selectedToolbar, value);
    } else if (name == "lastSavePath") {
        setParsed(this->lastSavePath, value);
    } else if (name == "lastOpenPath") {
        setParsed(this->lastOpenPath, value);
    } else if (name == "lastImagePath") {
        setParsed(this->lastImagePath, value);
    } else if (name == "edgePanSpeed") {
        setParsed(this->edgePanSpeed, value);
    } else if (name == "edgePanMaxMult") {
        setParsed(this->edgePanMaxMult, value);
    } else if (name == "zoomStep") {
        setParsed(this->zoomStep, value);
    } else if (name == "zoomStepScroll") {
        setParsed(this->zoomStepScroll, value);
    } else if (name == "displayDpi") {
        setParsed(this->displayDpi, value);
    } else if (name == "mainWndWidth") {
        setParsed(this->mainWndWidth, value);
    } else if (name == "mainWndHeight") {
        setParsed(this->mainWndHeight, value);
    } else if (name == "maximized") {
        setParsed(this->maximized, value);
    } else if (name == "showToolbar") {
        setParsed(this->showToolbar, value);
    } else if (name == "filepathShownInTitlebar") {
        setParsed(this->filepathShownInTitlebar, value);
    } else if (name == "pageNumberShownInTitlebar") {
        setParsed(this->pageNumberShownInTitlebar, value);
    } else if (name == "showSidebar") {
        setParsed(this->showSidebar, value);
    } else if (name == "sidebarNumberingStyle") {
        auto num = parse<int>(value);
        if (num < static_cast<int>(SidebarNumberingStyle::MIN) || static_cast<int>(SidebarNumberingStyle::MAX) < num) {
            num = static_cast<int>(SidebarNumberingStyle::DEFAULT);
            g_warning("Settings::Invalid sidebarNumberingStyle value. Reset to default.");
        }
        this->sidebarNumberingStyle = static_cast<SidebarNumberingStyle>(num);
    } else if (name == "sidebarWidth") {
        this->sidebarWidth = std::max<int>(parse<int>(value), 50);
    } else if (name == "sidebarOnRight") {
        setParsed(this->sidebarOnRight, value);
    } else if (name == "scrollbarOnLeft") {
        setParsed(this->scrollbarOnLeft, value);
    } else if (name == "menubarVisible") {
        setParsed(this->menubarVisible, value);
    } else if (name == "numColumns") {
        setParsed(this->numColumns, value);
    } else if (name == "numRows") {
        setParsed(this->numRows, value);
    } else if (name == "viewFixedRows") {
        setParsed(this->viewFixedRows, value);
    } else if (name == "layoutVertical") {
        setParsed(this->layoutVertical, value);
    } else if (name == "layoutRightToLeft") {
        setParsed(this->layoutRightToLeft, value);
    } else if (name == "layoutBottomToTop") {
        setParsed(this->layoutBottomToTop, value);
    } else if (name == "showPairedPages") {
        setParsed(this->showPairedPages, value);
    } else if (name == "numPairsOffset") {
        setParsed(this->numPairsOffset, value);
    } else if (name == "presentationMode") {
        setParsed(this->presentationMode, value);
    } else if (name == "autoloadMostRecent") {
        setParsed(this->autoloadMostRecent, value);
    } else if (name == "autoloadPdfXoj") {
        setParsed(this->autoloadPdfXoj, value);
    } else if (name == "stylusCursorType") {
        this->stylusCursorType = stylusCursorTypeFromString(parse<string>(value));
    } else if (name == "eraserVisibility") {
        this->eraserVisibility = eraserVisibilityFromString(parse<string>(value));
    } else if (name == "iconTheme") {
        this->iconTheme = iconThemeFromString(parse<string>(value));
    } else if (name == "themeVariant") {
        this->themeVariant = themeVariantFromString(parse<string>(value));
    } else if (name == "highlightPosition") {
        setParsed(this->highlightPosition, value);
    } else if (name == "cursorHighlightColor") {
        setParsed(this->cursorHighlightColor, value);
    } else if (name == "cursorHighlightRadius") {
        setParsed(this->cursorHighlightRadius, value);
    } else if (name == "cursorHighlightBorderColor") {
        setParsed(this->cursorHighlightBorderColor, value);
    } else if (name == "cursorHighlightBorderWidth") {
        setParsed(this->cursorHighlightBorderWidth, value);
    } else if (name == "useStockIcons") {
        setParsed(this->useStockIcons, value);
    } else if (name == "defaultSaveName") {
        setParsed(this->defaultSaveName, value);
    } else if (name == "defaultPdfExportName") {
        setParsed(this->defaultPdfExportName, value);
    } else if (name == "pluginEnabled") {
        setParsed(this->pluginEnabled, value);
    } else if (name == "pluginDisabled") {
        setParsed(this->pluginDisabled, value);
    } else if (name == "pageTemplate") {
        setParsed(this->pageTemplate, value);
    } else if (name == "sizeUnit") {
        setParsed(this->sizeUnit, value);
    } else if (name == "audioFolder") {
        setParsed(this->audioFolder, value);
    } else if (name == "autosaveEnabled") {
        setParsed(this->autosaveEnabled, value);
    } else if (name == "autosaveTimeout") {
        setParsed(this->autosaveTimeout, value);
    } else if (name == "defaultViewModeAttributes") {
        this->viewModes.at(VIEW_MODE_DEFAULT) = settingsStringToViewMode(parse<string>(value));
    } else if (name == "fullscreenViewModeAttributes") {
        this->viewModes.at(VIEW_MODE_FULLSCREEN) = settingsStringToViewMode(parse<string>(value));
    } else if (name == "presentationViewModeAttributes") {
        this->viewModes.at(VIEW_MODE_PRESENTATION) = settingsStringToViewMode(parse<string>(value));
    } else if (name == "touchZoomStartThreshold") {
        setParsed(this->touchZoomStartThreshold, value);
    } else if (name == "pageRerenderThreshold") {
        setParsed(this->pageRerenderThreshold, value);
    } else if (name == "pdfPageCacheSize") {
        setParsed(this->pdfPageCacheSize, value);
    } else if (name == "preloadPagesBefore") {
        setParsed(this->preloadPagesBefore, value);
    } else if (name == "preloadPagesAfter") {
        setParsed(this->preloadPagesAfter, value);
    } else if (name == "eagerPageCleanup") {
        setParsed(this->eagerPageCleanup, value);
    } else if (name == "selectionBorderColor") {
        setParsed(this->selectionBorderColor, value);
    } else if (name == "selectionMarkerColor") {
        setParsed(this->selectionMarkerColor, value);
    } else if (name == "activeSelectionColor") {
        setParsed(this->activeSelectionColor, value);
    } else if (name == "recolor.enabled") {
        setParsed(this->recolorParameters.recolorizeMainView, value);
    } else if (name == "recolor.sidebar") {
        setParsed(this->recolorParameters.recolorizeSidebarMiniatures, value);
    } else if (name == "recolor.light") {
        this->recolorParameters.recolor = Recolor(parse<Color>(value), this->recolorParameters.recolor.getDark());
    } else if (name == "recolor.dark") {
        this->recolorParameters.recolor = Recolor(this->recolorParameters.recolor.getLight(), parse<Color>(value));
    } else if (name == "backgroundColor") {
        setParsed(this->backgroundColor, value);
    } else if (name == "addHorizontalSpace") {
        setParsed(this->addHorizontalSpace, value);
    } else if (name == "addHorizontalSpaceAmount") {
        // const int oldHorizontalAmount = parse<int>(value);
        setParsed(this->addHorizontalSpaceAmountLeft, value);
        setParsed(this->addHorizontalSpaceAmountRight, value);
    } else if (name == "addHorizontalSpaceAmountRight") {
        setParsed(this->addHorizontalSpaceAmountRight, value);
    } else if (name == "addVerticalSpace") {
        setParsed(this->addVerticalSpace, value);
    } else if (name == "addVerticalSpaceAmount") {
        // const int oldVerticalAmount = parse<int>(value);
        setParsed(this->addHorizontalSpaceAmountLeft, value);
        setParsed(this->addHorizontalSpaceAmountRight, value);
    } else if (name == "addVerticalSpaceAmountAbove") {
        setParsed(this->addVerticalSpaceAmountAbove, value);
    } else if (name == "addHorizontalSpaceAmountLeft") {
        setParsed(this->addHorizontalSpaceAmountLeft, value);
    } else if (name == "addVerticalSpaceAmountBelow") {
        setParsed(this->addVerticalSpaceAmountBelow, value);
    } else if (name == "unlimitedScrolling") {
        setParsed(this->unlimitedScrolling, value);
    } else if (name == "drawDirModsEnabled") {
        setParsed(this->drawDirModsEnabled, value);
    } else if (name == "drawDirModsRadius") {
        setParsed(this->drawDirModsRadius, value);
    } else if (name == "snapRotation") {
        setParsed(this->snapRotation, value);
    } else if (name == "snapRotationTolerance") {
        setParsed(this->snapRotationTolerance, value);
    } else if (name == "snapGrid") {
        setParsed(this->snapGrid, value);
    } else if (name == "snapGridSize") {
        setParsed(this->snapGridSize, value);
    } else if (name == "snapGridTolerance") {
        setParsed(this->snapGridTolerance, value);
    } else if (name == "strokeRecognizerMinSize") {
        setParsed(this->strokeRecognizerMinSize, value);
    } else if (name == "touchDrawing") {
        setParsed(this->touchDrawing, value);
    } else if (name == "gtkTouchInertialScrolling") {
        setParsed(this->gtkTouchInertialScrolling, value);
    } else if (name == "pressureGuessing") {
        setParsed(this->pressureGuessing, value);
    } else if (name == "scrollbarHideType") {
        if (value == "both") {
            this->scrollbarHideType = SCROLLBAR_HIDE_BOTH;
        } else if (value == "horizontal") {
            this->scrollbarHideType = SCROLLBAR_HIDE_HORIZONTAL;
        } else if (value == "vertical") {
            this->scrollbarHideType = SCROLLBAR_HIDE_VERTICAL;
        } else {
            this->scrollbarHideType = SCROLLBAR_HIDE_NONE;
        }
    } else if (name == "disableScrollbarFadeout") {
        setParsed(this->disableScrollbarFadeout, value);
    } else if (name == "disableAudio") {
        setParsed(this->disableAudio, value);
#ifdef ENABLE_AUDIO
    } else if (name == "audioSampleRate") {
        setParsed(this->audioSampleRate, value);
    } else if (name == "audioGain") {
        setParsed(this->audioGain, value);
    } else if (name == "defaultSeekTime") {
        setParsed(this->defaultSeekTime, value);
    } else if (name == "audioInputDevice") {
        setParsed(this->audioInputDevice, value);
    } else if (name == "audioOutputDevice") {
        setParsed(this->audioOutputDevice, value);
#endif
    } else if (name == "numIgnoredStylusEvents") {
        this->numIgnoredStylusEvents = static_cast<int>(parse<unsigned int>(value));
    } else if (name == "inputSystemTPCButton") {
        setParsed(this->inputSystemTPCButton, value);
    } else if (name == "inputSystemDrawOutsideWindow") {
        setParsed(this->inputSystemDrawOutsideWindow, value);
    } else if (name == "emptyLastPageAppend") {
        this->emptyLastPageAppend = emptyLastPageAppendFromString(parse<string>(value));
    } else if (name == "strokeFilterIgnoreTime") {
        setParsed(this->strokeFilterIgnoreTime, value);
    } else if (name == "strokeFilterIgnoreLength") {
        setParsed(this->strokeFilterIgnoreLength, value);
    } else if (name == "strokeFilterSuccessiveTime") {
        setParsed(this->strokeFilterSuccessiveTime, value);
    } else if (name == "strokeFilterEnabled") {
        setParsed(this->strokeFilterEnabled, value);
    } else if (name == "doActionOnStrokeFiltered") {
        setParsed(this->doActionOnStrokeFiltered, value);
    } else if (name == "trySelectOnStrokeFiltered") {
        setParsed(this->trySelectOnStrokeFiltered, value);
    } else if (name == "latexSettings.autoCheckDependencies") {
        setParsed(this->latexSettings.autoCheckDependencies, value);
    } else if (name == "latexSettings.defaultText") {
        setParsed(this->latexSettings.defaultText, value);
    } else if (name == "latexSettings.globalTemplatePath") {
        setParsed(this->latexSettings.globalTemplatePath, value);
    } else if (name == "latexSettings.genCmd") {
        setParsed(this->latexSettings.genCmd, value);
    } else if (name == "latexSettings.sourceViewThemeId") {
        setParsed(this->latexSettings.sourceViewThemeId, value);
    } else if (name == "latexSettings.editorFont") {
        this->latexSettings.editorFont = newFont;
    } else if (name == "latexSettings.useCustomEditorFont") {
        setParsed(this->latexSettings.useCustomEditorFont, value);
    } else if (name == "latexSettings.editorWordWrap") {
        setParsed(this->latexSettings.editorWordWrap, value);
    } else if (name == "latexSettings.sourceViewAutoIndent") {
        setParsed(this->latexSettings.sourceViewAutoIndent, value);
    } else if (name == "latexSettings.sourceViewSyntaxHighlight") {
        setParsed(this->latexSettings.sourceViewSyntaxHighlight, value);
    } else if (name == "latexSettings.sourceViewShowLineNumbers") {
        setParsed(this->latexSettings.sourceViewShowLineNumbers, value);
    } else if (name == "snapRecognizedShapesEnabled") {
        setParsed(this->snapRecognizedShapesEnabled, value);
    } else if (name == "restoreLineWidthEnabled") {
        setParsed(this->restoreLineWidthEnabled, value);
    } else if (name == "preferredLocale") {
        setParsed(this->preferredLocale, value);
    } else if (name == "useSpacesForTab") {
        this->setUseSpacesAsTab(parse<bool>(value));
    } else if (name == "numberOfSpacesForTab") {
        this->setNumberOfSpacesForTab(parse<unsigned int>(value));
        /**
         * Stabilizer related settings
         */
    } else if (name == "stabilizerAveragingMethod") {
        this->stabilizerAveragingMethod = static_cast<StrokeStabilizer::AveragingMethod>(parse<int>(value));
    } else if (name == "stabilizerPreprocessor") {
        this->stabilizerPreprocessor = static_cast<StrokeStabilizer::Preprocessor>(parse<int>(value));
    } else if (name == "stabilizerBuffersize") {
        setParsed(this->stabilizerBuffersize, value);
    } else if (name == "stabilizerSigma") {
        setParsed(this->stabilizerSigma, value);
    } else if (name == "stabilizerDeadzoneRadius") {
        setParsed(this->stabilizerDeadzoneRadius, value);
    } else if (name == "stabilizerDrag") {
        setParsed(this->stabilizerDrag, value);
    } else if (name == "stabilizerMass") {
        setParsed(this->stabilizerMass, value);
    } else if (name == "stabilizerCuspDetection") {
        setParsed(this->stabilizerCuspDetection, value);
    } else if (name == "stabilizerFinalizeStroke") {
        setParsed(this->stabilizerFinalizeStroke, value);
    } else if (name == "colorPalette") {
        setParsed(this->colorPaletteSetting, value);
    }
}

void Settings::loadDeviceClasses() {
    SElement& s = getCustomElement("deviceClasses");
    for (auto device: s.children()) {
        SElement& deviceNode = device.second;
        int deviceClass = 0;
        int deviceSource = 0;
        deviceNode.getInt("deviceClass", deviceClass);
        deviceNode.getInt("deviceSource", deviceSource);
        auto devClass = static_cast<InputDeviceTypeOption>(deviceClass);
        if (devClass == InputDeviceTypeOption::MouseKeyboardCombo) {
            // This extra class is no longer handled differently from Mouse. Merge them.
            devClass = InputDeviceTypeOption::Mouse;
        }
        inputDeviceClasses.emplace(device.first, std::make_pair(devClass, static_cast<GdkInputSource>(deviceSource)));
    }
}

void Settings::loadButtonConfig() {
    SElement& s = getCustomElement("buttonConfig");

    for (int i = 0; i < BUTTON_COUNT; i++) {
        SElement& e = s.child(buttonToString(static_cast<Button>(i)));
        const auto& cfg = buttonConfig[i];

        string sType;
        if (e.getString("tool", sType)) {
            const ToolType type = toolTypeFromString(sType);
            cfg->action = type;

            if (type == TOOL_PEN) {
                string strokeType;
                cfg->strokeType =
                        e.getString("strokeType", strokeType) ? strokeTypeFromString(strokeType) : STROKE_TYPE_NONE;
            }

            if (type == TOOL_PEN || type == TOOL_HIGHLIGHTER) {
                if (string drawingType; e.getString("drawingType", drawingType)) {
                    cfg->drawingType = drawingTypeFromString(drawingType);
                }

                if (string sSize; e.getString("size", sSize)) {
                    cfg->size = toolSizeFromString(sSize);
                } else {
                    // If not specified: do not change
                    cfg->size = TOOL_SIZE_NONE;
                }
            }

            if (type == TOOL_PEN || type == TOOL_HIGHLIGHTER || type == TOOL_TEXT) {
                if (int iColor; e.getInt("color", iColor)) {
                    cfg->color = Color(as_unsigned(iColor));
                }
            }

            if (type == TOOL_ERASER) {
                if (string sEraserMode; e.getString("eraserMode", sEraserMode)) {
                    cfg->eraserMode = eraserTypeFromString(sEraserMode);
                } else {
                    // If not specified: do not change
                    cfg->eraserMode = ERASER_TYPE_NONE;
                }

                if (string sSize; e.getString("size", sSize)) {
                    cfg->size = toolSizeFromString(sSize);
                } else {
                    // If not specified: do not change
                    cfg->size = TOOL_SIZE_NONE;
                }
            }

            // Touch device
            if (i == BUTTON_TOUCH) {
                if (!e.getString("device", cfg->device)) {
                    cfg->device = "";
                }

                e.getBool("disableDrawing", cfg->disableDrawing);
            }
        }
    }
}

auto Settings::load() -> bool {
    xmlKeepBlanksDefault(0);

    if (!fs::exists(filepath)) {
        g_warning("Settings file %s does not exist. Regenerating. ", filepath.string().c_str());
        save();
    }

    xmlDocPtr doc = xmlParseFile(char_cast(filepath.u8string().c_str()));

    if (doc == nullptr) {
        g_warning("Settings::load:: doc == null, could not load Settings!\n");
        return false;
    }

    xmlNodePtr cur = xmlDocGetRootElement(doc);
    if (cur == nullptr) {
        g_message("The settings file \"%s\" is empty", filepath.string().c_str());
        xmlFreeDoc(doc);

        return false;
    }

    if (cur->name != "settings"sv) {
        g_message("File \"%s\" is of the wrong type", filepath.string().c_str());
        xmlFreeDoc(doc);

        return false;
    }

    cur = xmlDocGetRootElement(doc);
    cur = cur->xmlChildrenNode;

    while (cur != nullptr) {
        parseItem(doc, cur);

        cur = cur->next;
    }

    xmlFreeDoc(doc);

    loadButtonConfig();
    loadDeviceClasses();

    // This must be done before the color palette to ensure the color names are translated properly
#ifdef _WIN32
    _putenv_s("LANGUAGE", this->preferredLocale.c_str());
#else
    setenv("LANGUAGE", this->preferredLocale.c_str(), 1);
#endif

    return true;
}

template <typename T>
xmlNodePtr Settings::saveProperty(const std::string& key, T value, xmlNodePtr parent) {
    const xmlNodePtr xmlNode = xmlNewChild(parent, nullptr, "property"_xml, nullptr);
    char buffer[20];

    std::string str{};

    if constexpr (std::is_same_v<T, int>) {
        const auto result = std::to_chars(buffer, buffer + sizeof(buffer), value);
        str = {buffer, result.ptr};
    } else if constexpr (std::is_same_v<T, uint32_t>) {
        const auto result = std::to_chars(buffer, buffer + sizeof(buffer), value);
        str = {buffer, result.ptr};
    } else if constexpr (std::is_same_v<T, double>) {
        const auto result = std::to_chars(buffer, buffer + sizeof(buffer), value);
        str = {buffer, result.ptr};
    } else if constexpr (std::is_same_v<T, bool>) {
        str = value ? "true" : "false";
    } else if constexpr (std::is_same_v<T, fs::path>) {
        str = char_cast(value.u8string().c_str());
    } else if constexpr (std::convertible_to<T, std::string>) {
        str = value;
    } else if constexpr (std::convertible_to<T, std::u8string>) {
        str = char_cast(value);
    }

    if constexpr (std::is_same_v<T, XojFont>) {
        xmlSetProp(xmlNode, "font"_xml, reinterpret_cast<const xmlChar*>(value.getName().c_str()));

        char sSize[G_ASCII_DTOSTR_BUF_SIZE];

        g_ascii_formatd(sSize, G_ASCII_DTOSTR_BUF_SIZE, Util::PRECISION_FORMAT_STRING,
                        value.getSize());  // no locale
        xmlSetProp(xmlNode, "size"_xml, reinterpret_cast<const xmlChar*>(sSize));
    } else {
        xmlSetProp(xmlNode, "value"_xml, reinterpret_cast<const xmlChar*>(str.c_str()));
    }

    xmlSetProp(xmlNode, "name"_xml, reinterpret_cast<const xmlChar*>(key.c_str()));
    return xmlNode;
}

void Settings::saveDeviceClasses() {
    SElement& s = getCustomElement("deviceClasses");

    for (auto& device: inputDeviceClasses) {
        const std::string& name = device.first;
        InputDeviceTypeOption& deviceClass = device.second.first;
        GdkInputSource& source = device.second.second;
        SElement& e = s.child(name);
        e.set("deviceClass", static_cast<int>(deviceClass));
        e.set("deviceSource", source);
    }
}

void Settings::saveButtonConfig() {
    SElement& s = getCustomElement("buttonConfig");
    s.clear();

    for (int i = 0; i < BUTTON_COUNT; i++) {
        SElement& e = s.child(buttonToString(static_cast<Button>(i)));
        const auto& cfg = buttonConfig[i];

        ToolType const type = cfg->action;
        e.set("tool", toolTypeToString(type).data());

        if (type == TOOL_PEN) {
            e.set("strokeType", strokeTypeToString(cfg->strokeType).data());
        }

        if (type == TOOL_PEN || type == TOOL_HIGHLIGHTER) {
            e.set("drawingType", drawingTypeToString(cfg->drawingType).data());
            e.set("size", toolSizeToString(cfg->size).data());
        }

        if (type == TOOL_PEN || type == TOOL_HIGHLIGHTER || type == TOOL_TEXT) {
            e.set("color", static_cast<uint32_t>(cfg->color));
        }

        if (type == TOOL_ERASER) {
            e.set("eraserMode", eraserTypeToString(cfg->eraserMode).data());
            e.set("size", toolSizeToString(cfg->size).data());
        }

        // Touch device
        if (i == BUTTON_TOUCH) {
            e.set("device", cfg->device);
            e.set("disableDrawing", cfg->disableDrawing);
        }
    }
}

/**
 * Do not save settings until transactionEnd() is called
 */
void Settings::transactionStart() { inTransaction = true; }

/**
 * Stop transaction and save settings
 */
void Settings::transactionEnd() {
    inTransaction = false;
    save();
}

void Settings::save() {
    if (inTransaction) {
        return;
    }

    xmlDocPtr doc = nullptr;
    xmlNodePtr root = nullptr;
    xmlNodePtr xmlNode = nullptr;

    xmlIndentTreeOutput = true;

    doc = xmlNewDoc("1.0"_xml);
    if (doc == nullptr) {
        return;
    }

    saveButtonConfig();
    saveDeviceClasses();

    /* Create metadata root */
    root = xmlNewDocNode(doc, nullptr, "settings"_xml, nullptr);
    xmlDocSetRootElement(doc, root);
    xmlNodePtr com = xmlNewComment("The Xournal++ settings file. Do not edit this file! "
                                   "Most settings are available in the Settings dialog, "
                                   "the others are commented in this file, but handle with care!"_xml);
    xmlAddPrevSibling(root, com);

    SAVE_PROP(pressureSensitivity);
    SAVE_PROP(minimumPressure);
    SAVE_PROP(pressureMultiplier);

    SAVE_PROP(zoomGesturesEnabled);

    SAVE_PROP(selectedToolbar);

    SAVE_PROP(lastSavePath);

    // saveProperty("lastSavePath", char_cast(this->lastSavePath.u8string().c_str()), root);
    saveProperty("lastOpenPath", char_cast(this->lastOpenPath.u8string().c_str()), root);
    saveProperty("lastImagePath", char_cast(this->lastImagePath.u8string().c_str()), root);

    SAVE_PROP(edgePanSpeed);
    SAVE_PROP(edgePanMaxMult);
    SAVE_PROP(zoomStep);
    SAVE_PROP(zoomStepScroll);
    SAVE_PROP(displayDpi);
    SAVE_PROP(mainWndWidth);
    SAVE_PROP(mainWndHeight);
    SAVE_PROP(maximized);

    SAVE_PROP(showToolbar);

    SAVE_PROP(showSidebar);
    SAVE_PROP(sidebarWidth);
    xmlNode = saveProperty("sidebarNumberingStyle", static_cast<int>(sidebarNumberingStyle), root);

    SAVE_PROP(sidebarOnRight);
    SAVE_PROP(scrollbarOnLeft);
    SAVE_PROP(menubarVisible);
    SAVE_PROP(filepathShownInTitlebar);
    SAVE_PROP(pageNumberShownInTitlebar);
    SAVE_PROP(numColumns);
    SAVE_PROP(numRows);
    SAVE_PROP(viewFixedRows);
    SAVE_PROP(showPairedPages);
    SAVE_PROP(layoutVertical);
    SAVE_PROP(layoutRightToLeft);
    SAVE_PROP(layoutBottomToTop);
    SAVE_PROP(numPairsOffset);
    xmlNode = saveProperty("emptyLastPageAppend", emptyLastPageAppendToString(this->emptyLastPageAppend), root);
    ATTACH_COMMENT("The icon theme, allowed values are \"disabled\", \"onDrawOfLastPage\", and \"onScrollOfLastPage\"");
    SAVE_PROP(presentationMode);

    auto defaultViewModeAttributes = viewModeToSettingsString(viewModes.at(PresetViewModeIds::VIEW_MODE_DEFAULT));
    auto fullscreenViewModeAttributes = viewModeToSettingsString(viewModes.at(PresetViewModeIds::VIEW_MODE_FULLSCREEN));
    auto presentationViewModeAttributes =
            viewModeToSettingsString(viewModes.at(PresetViewModeIds::VIEW_MODE_PRESENTATION));
    SAVE_PROP(defaultViewModeAttributes);
    ATTACH_COMMENT("Which GUI elements are shown in default view mode, separated by a colon (,)");
    SAVE_PROP(fullscreenViewModeAttributes);
    ATTACH_COMMENT("Which GUI elements are shown in fullscreen view mode, separated by a colon (,)");
    SAVE_PROP(presentationViewModeAttributes);
    ATTACH_COMMENT("Which GUI elements are shown in presentation view mode, separated by a colon (,)");

    xmlNode = saveProperty("stylusCursorType", stylusCursorTypeToString(this->stylusCursorType), root);
    ATTACH_COMMENT("The cursor icon used with a stylus, allowed values are \"none\", \"dot\", \"big\", \"arrow\"");

    xmlNode = saveProperty("eraserVisibility", eraserVisibilityToString(this->eraserVisibility), root);
    ATTACH_COMMENT("The eraser cursor visibility used with a stylus, allowed values are \"never\", \"always\", "
                   "\"hover\", \"touch\"");

    xmlNode = saveProperty("iconTheme", iconThemeToString(this->iconTheme), root);
    ATTACH_COMMENT("The icon theme, allowed values are \"iconsColor\", \"iconsLucide\"");

    xmlNode = saveProperty("themeVariant", themeVariantToString(this->themeVariant), root);
    ATTACH_COMMENT("Dark/light mode, allowed values are \"useSystem\", \"forceLight\", \"forceDark\"");

    SAVE_PROP(highlightPosition);
    xmlNode = saveProperty("cursorHighlightColor", static_cast<uint32_t>(cursorHighlightColor), root);
    xmlNode = saveProperty("cursorHighlightBorderColor", static_cast<uint32_t>(cursorHighlightBorderColor), root);
    SAVE_PROP(cursorHighlightRadius);
    SAVE_PROP(cursorHighlightBorderWidth);
    SAVE_PROP(useStockIcons);

    SAVE_PROP(disableScrollbarFadeout);
    SAVE_PROP(disableAudio);

    if (this->scrollbarHideType == SCROLLBAR_HIDE_BOTH) {
        xmlNode = saveProperty("scrollbarHideType", "both", root);
    } else if (this->scrollbarHideType == SCROLLBAR_HIDE_HORIZONTAL) {
        xmlNode = saveProperty("scrollbarHideType", "horizontal", root);
    } else if (this->scrollbarHideType == SCROLLBAR_HIDE_VERTICAL) {
        xmlNode = saveProperty("scrollbarHideType", "vertical", root);
    } else {
        xmlNode = saveProperty("scrollbarHideType", "none", root);
    }
    ATTACH_COMMENT("Hides scroolbars in the main window, allowed values: \"none\", \"horizontal\", \"vertical\", "
                   "\"both\"");

    SAVE_PROP(autoloadMostRecent);
    SAVE_PROP(autoloadPdfXoj);
    SAVE_PROP(defaultSaveName);
    SAVE_PROP(defaultPdfExportName);

    SAVE_PROP(autosaveEnabled);
    SAVE_PROP(autosaveTimeout);

    SAVE_PROP(addHorizontalSpace);
    SAVE_PROP(addHorizontalSpaceAmountRight);
    SAVE_PROP(addHorizontalSpaceAmountLeft);
    SAVE_PROP(addVerticalSpace);
    SAVE_PROP(addVerticalSpaceAmountAbove);
    SAVE_PROP(addVerticalSpaceAmountBelow);

    SAVE_PROP(unlimitedScrolling);

    SAVE_PROP(drawDirModsEnabled);
    SAVE_PROP(drawDirModsRadius);


    SAVE_PROP(snapRotation);
    SAVE_PROP(snapRotationTolerance);
    SAVE_PROP(snapGrid);
    SAVE_PROP(snapGridTolerance);
    SAVE_PROP(snapGridSize);

    SAVE_PROP(strokeRecognizerMinSize);

    SAVE_PROP(touchDrawing);
    SAVE_PROP(gtkTouchInertialScrolling);
    SAVE_PROP(pressureGuessing);

    xmlNode = saveProperty("recolor.enabled", recolorParameters.recolorizeMainView ? "true" : "false", root);
    xmlNode = saveProperty("recolor.sidebar", recolorParameters.recolorizeSidebarMiniatures ? "true" : "false", root);
    xmlNode = saveProperty("recolor.dark", static_cast<uint32_t>(recolorParameters.recolor.getDark()), root);
    xmlNode = saveProperty("recolor.light", static_cast<uint32_t>(recolorParameters.recolor.getLight()), root);

    xmlNode = saveProperty("selectionBorderColor", static_cast<uint32_t>(selectionBorderColor), root);
    xmlNode = saveProperty("backgroundColor", static_cast<uint32_t>(backgroundColor), root);
    xmlNode = saveProperty("selectionMarkerColor", static_cast<uint32_t>(selectionMarkerColor), root);
    xmlNode = saveProperty("activeSelectionColor", static_cast<uint32_t>(activeSelectionColor), root);

    SAVE_PROP(touchZoomStartThreshold);
    SAVE_PROP(pageRerenderThreshold);

    SAVE_PROP(pdfPageCacheSize);
    ATTACH_COMMENT("The count of rendered PDF pages which will be cached.");
    SAVE_PROP(preloadPagesBefore);
    SAVE_PROP(preloadPagesAfter);
    SAVE_PROP(eagerPageCleanup);

    SAVE_PROP(pageTemplate);
    ATTACH_COMMENT("Config for new pages");

    SAVE_PROP(sizeUnit);

#ifdef ENABLE_AUDIO
    SAVE_PROP(audioFolder);
    SAVE_PROP(audioInputDevice);
    SAVE_PROP(audioOutputDevice);
    SAVE_PROP(audioSampleRate);
    SAVE_PROP(audioGain);
    SAVE_PROP(defaultSeekTime);
#endif

    SAVE_PROP(pluginEnabled);
    SAVE_PROP(pluginDisabled);

    SAVE_PROP(strokeFilterIgnoreTime);
    SAVE_PROP(strokeFilterIgnoreLength);
    SAVE_PROP(strokeFilterSuccessiveTime);
    SAVE_PROP(strokeFilterEnabled);
    SAVE_PROP(doActionOnStrokeFiltered);
    SAVE_PROP(trySelectOnStrokeFiltered);

    SAVE_PROP(snapRecognizedShapesEnabled);
    SAVE_PROP(restoreLineWidthEnabled);

    SAVE_PROP(numIgnoredStylusEvents);

    SAVE_PROP(inputSystemTPCButton);
    SAVE_PROP(inputSystemDrawOutsideWindow);

    SAVE_PROP(preferredLocale);

    SAVE_PROP(useSpacesForTab);
    SAVE_PROP(numberOfSpacesForTab);

    SAVE_PROP(laserPointerFadeOutTime);

    /**
     * Stabilizer related settings
     */
    saveProperty("stabilizerAveragingMethod", static_cast<int>(stabilizerAveragingMethod), root);
    saveProperty("stabilizerPreprocessor", static_cast<int>(stabilizerPreprocessor), root);
    SAVE_PROP(stabilizerBuffersize);
    SAVE_PROP(stabilizerSigma);
    SAVE_PROP(stabilizerDeadzoneRadius);
    SAVE_PROP(stabilizerDrag);
    SAVE_PROP(stabilizerMass);
    SAVE_PROP(stabilizerCuspDetection);
    SAVE_PROP(stabilizerFinalizeStroke);

    if (!this->colorPaletteSetting.empty()) {
        saveProperty("colorPalette", char_cast(this->colorPaletteSetting.u8string().c_str()), root);
    }

    SAVE_PROP(colorPaletteSetting);

    /**/

    SAVE_PROP(latexSettings.autoCheckDependencies);
    SAVE_PROP(latexSettings.defaultText);
    // Inline SAVE_PROP(latexSettings.globalTemplatePath) since it
    // breaks on Windows due to the native character representation being
    // wchar_t instead of char
    fs::path& p = latexSettings.globalTemplatePath;
    xmlNode = saveProperty("latexSettings.globalTemplatePath", p.empty() ? "" : char_cast(p.u8string().c_str()), root);
    SAVE_PROP(latexSettings.genCmd);
    SAVE_PROP(latexSettings.sourceViewThemeId);
    SAVE_PROP(latexSettings.editorFont);
    SAVE_PROP(latexSettings.useCustomEditorFont);
    SAVE_PROP(latexSettings.editorWordWrap);
    SAVE_PROP(latexSettings.sourceViewAutoIndent);
    SAVE_PROP(latexSettings.sourceViewSyntaxHighlight);
    SAVE_PROP(latexSettings.sourceViewShowLineNumbers);
    SAVE_PROP(latexSettings.useExternalEditor);
    SAVE_PROP(latexSettings.externalEditorAutoConfirm);
    SAVE_PROP(latexSettings.externalEditorCmd);
    SAVE_PROP(latexSettings.temporaryFileExt);

    SAVE_PROP(font);

    for (std::map<string, SElement>::value_type p: data) {
        saveData(root, p.first, p.second);
    }

    xmlSaveFormatFileEnc(char_cast(filepath.u8string().c_str()), doc, "UTF-8", 1);
    xmlFreeDoc(doc);
}

void Settings::saveData(xmlNodePtr root, const string& name, SElement& elem) {
    xmlNodePtr xmlNode = xmlNewChild(root, nullptr, "data"_xml, nullptr);

    xmlSetProp(xmlNode, "name"_xml, reinterpret_cast<const xmlChar*>(name.c_str()));

    for (auto const& [aname, attrib]: elem.attributes()) {
        string type;
        string value;

        if (attrib.type == ATTRIBUTE_TYPE_BOOLEAN) {
            type = "boolean";

            if (attrib.iValue) {
                value = "true";
            } else {
                value = "false";
            }
        } else if (attrib.type == ATTRIBUTE_TYPE_INT) {
            type = "int";

            char* tmp = g_strdup_printf("%i", attrib.iValue);
            value = tmp;
            g_free(tmp);
        } else if (attrib.type == ATTRIBUTE_TYPE_DOUBLE) {
            type = "double";

            char tmp[G_ASCII_DTOSTR_BUF_SIZE];
            g_ascii_formatd(tmp, G_ASCII_DTOSTR_BUF_SIZE, Util::PRECISION_FORMAT_STRING, attrib.dValue);
            value = tmp;
        } else if (attrib.type == ATTRIBUTE_TYPE_INT_HEX) {
            type = "hex";

            char* tmp = g_strdup_printf("%06x", attrib.iValue);
            value = tmp;
            g_free(tmp);
        } else if (attrib.type == ATTRIBUTE_TYPE_STRING) {
            type = "string";
            value = attrib.sValue;
        } else {
            // Unknown type or empty attribute
            continue;
        }

        xmlNodePtr at = nullptr;
        at = xmlNewChild(xmlNode, nullptr, "attribute"_xml, nullptr);

        xmlSetProp(at, "name"_xml, reinterpret_cast<const xmlChar*>(aname.c_str()));
        xmlSetProp(at, "type"_xml, reinterpret_cast<const xmlChar*>(type.c_str()));
        xmlSetProp(at, "value"_xml, reinterpret_cast<const xmlChar*>(value.c_str()));

        if (!attrib.comment.empty()) {
            xmlNodePtr com = xmlNewComment(reinterpret_cast<const xmlChar*>(attrib.comment.c_str()));
            xmlAddPrevSibling(xmlNode, com);
        }
    }

    for (std::map<string, SElement>::value_type p: elem.children()) {
        saveData(xmlNode, p.first, p.second);
    }
}

// Getter- / Setter
auto Settings::isPressureSensitivity() const -> bool { return this->pressureSensitivity; }

auto Settings::isZoomGesturesEnabled() const -> bool { return this->zoomGesturesEnabled; }

void Settings::setZoomGesturesEnabled(bool enable) {
    if (this->zoomGesturesEnabled == enable) {
        return;
    }
    this->zoomGesturesEnabled = enable;
    save();
}

auto Settings::isSidebarOnRight() const -> bool { return this->sidebarOnRight; }

void Settings::setSidebarOnRight(bool right) {
    if (this->sidebarOnRight == right) {
        return;
    }

    this->sidebarOnRight = right;

    save();
}

auto Settings::isScrollbarOnLeft() const -> bool { return this->scrollbarOnLeft; }

void Settings::setScrollbarOnLeft(bool right) {
    if (this->scrollbarOnLeft == right) {
        return;
    }

    this->scrollbarOnLeft = right;

    save();
}

auto Settings::isMenubarVisible() const -> bool { return this->menubarVisible; }

void Settings::setMenubarVisible(bool visible) {
    if (this->menubarVisible == visible) {
        return;
    }

    this->menubarVisible = visible;

    save();
}

const bool Settings::isFilepathInTitlebarShown() const { return this->filepathShownInTitlebar; }

void Settings::setFilepathInTitlebarShown(const bool shown) {
    if (this->filepathShownInTitlebar == shown) {
        return;
    }

    this->filepathShownInTitlebar = shown;

    save();
}

const bool Settings::isPageNumberInTitlebarShown() const { return this->pageNumberShownInTitlebar; }

void Settings::setPageNumberInTitlebarShown(const bool shown) {
    if (this->pageNumberShownInTitlebar == shown) {
        return;
    }

    this->pageNumberShownInTitlebar = shown;

    save();
}

auto Settings::getAutosaveTimeout() const -> int { return this->autosaveTimeout; }

void Settings::setAutosaveTimeout(int autosave) {
    if (this->autosaveTimeout == autosave) {
        return;
    }

    this->autosaveTimeout = autosave;

    save();
}

auto Settings::isAutosaveEnabled() const -> bool { return this->autosaveEnabled; }

void Settings::setAutosaveEnabled(bool autosave) {
    if (this->autosaveEnabled == autosave) {
        return;
    }

    this->autosaveEnabled = autosave;

    save();
}

auto Settings::getAddVerticalSpace() const -> bool { return this->addVerticalSpace; }

void Settings::setAddVerticalSpace(bool space) { this->addVerticalSpace = space; }

auto Settings::getAddVerticalSpaceAmountAbove() const -> int { return this->addVerticalSpaceAmountAbove; }

void Settings::setAddVerticalSpaceAmountAbove(int pixels) {
    if (this->addVerticalSpaceAmountAbove == pixels) {
        return;
    }

    this->addVerticalSpaceAmountAbove = pixels;
}

auto Settings::getAddVerticalSpaceAmountBelow() const -> int { return this->addVerticalSpaceAmountBelow; }

void Settings::setAddVerticalSpaceAmountBelow(int pixels) {
    if (this->addVerticalSpaceAmountBelow == pixels) {
        return;
    }

    this->addVerticalSpaceAmountBelow = pixels;
}


auto Settings::getAddHorizontalSpace() const -> bool { return this->addHorizontalSpace; }

void Settings::setAddHorizontalSpace(bool space) { this->addHorizontalSpace = space; }

auto Settings::getAddHorizontalSpaceAmountRight() const -> int { return this->addHorizontalSpaceAmountRight; }

void Settings::setAddHorizontalSpaceAmountRight(int pixels) {
    if (this->addHorizontalSpaceAmountRight == pixels) {
        return;
    }

    this->addHorizontalSpaceAmountRight = pixels;
}

auto Settings::getAddHorizontalSpaceAmountLeft() const -> int { return this->addHorizontalSpaceAmountLeft; }

void Settings::setAddHorizontalSpaceAmountLeft(int pixels) {
    if (this->addHorizontalSpaceAmountLeft == pixels) {
        return;
    }

    this->addHorizontalSpaceAmountLeft = pixels;
}

auto Settings::getUnlimitedScrolling() const -> bool { return this->unlimitedScrolling; }

void Settings::setUnlimitedScrolling(bool enable) {
    if (enable == this->unlimitedScrolling) {
        return;
    }

    this->unlimitedScrolling = enable;
}

auto Settings::getDrawDirModsEnabled() const -> bool { return this->drawDirModsEnabled; }

void Settings::setDrawDirModsEnabled(bool enable) { this->drawDirModsEnabled = enable; }

auto Settings::getDrawDirModsRadius() const -> int { return this->drawDirModsRadius; }

void Settings::setDrawDirModsRadius(int pixels) {
    if (this->drawDirModsRadius == pixels) {
        return;
    }

    this->drawDirModsRadius = pixels;
    save();
}

auto Settings::getStylusCursorType() const -> StylusCursorType { return this->stylusCursorType; }

void Settings::setStylusCursorType(StylusCursorType type) {
    if (this->stylusCursorType == type) {
        return;
    }

    this->stylusCursorType = type;

    save();
}

auto Settings::getEraserVisibility() const -> EraserVisibility { return this->eraserVisibility; }

void Settings::setEraserVisibility(EraserVisibility eraserVisibility) {
    if (this->eraserVisibility == eraserVisibility) {
        return;
    }

    this->eraserVisibility = eraserVisibility;

    save();
}

auto Settings::getIconTheme() const -> IconTheme { return this->iconTheme; }

void Settings::setIconTheme(IconTheme iconTheme) {
    if (this->iconTheme == iconTheme) {
        return;
    }

    this->iconTheme = iconTheme;

    save();
}

auto Settings::getThemeVariant() const -> ThemeVariant { return this->themeVariant; }

void Settings::setThemeVariant(ThemeVariant theme) {
    if (this->themeVariant == theme) {
        return;
    }
    this->themeVariant = theme;
    save();
}

auto Settings::getSidebarNumberingStyle() const -> SidebarNumberingStyle { return this->sidebarNumberingStyle; };

void Settings::setSidebarNumberingStyle(SidebarNumberingStyle numberingStyle) {
    if (this->sidebarNumberingStyle == numberingStyle) {
        return;
    }

    this->sidebarNumberingStyle = numberingStyle;

    save();
}

auto Settings::isHighlightPosition() const -> bool { return this->highlightPosition; }

void Settings::setHighlightPosition(bool highlight) {
    if (this->highlightPosition == highlight) {
        return;
    }

    this->highlightPosition = highlight;
    save();
}

auto Settings::getCursorHighlightColor() const -> Color { return this->cursorHighlightColor; }

void Settings::setCursorHighlightColor(Color color) {
    if (this->cursorHighlightColor != color) {
        this->cursorHighlightColor = color;
        save();
    }
}

auto Settings::getCursorHighlightRadius() const -> double { return this->cursorHighlightRadius; }

void Settings::setCursorHighlightRadius(double radius) {
    if (this->cursorHighlightRadius != radius) {
        this->cursorHighlightRadius = radius;
        save();
    }
}

auto Settings::getCursorHighlightBorderColor() const -> Color { return this->cursorHighlightBorderColor; }

void Settings::setCursorHighlightBorderColor(Color color) {
    if (this->cursorHighlightBorderColor != color) {
        this->cursorHighlightBorderColor = color;
        save();
    }
}

auto Settings::getCursorHighlightBorderWidth() const -> double { return this->cursorHighlightBorderWidth; }

void Settings::setCursorHighlightBorderWidth(double radius) {
    if (this->cursorHighlightBorderWidth != radius) {
        this->cursorHighlightBorderWidth = radius;
        save();
    }
}

auto Settings::isSnapRotation() const -> bool { return this->snapRotation; }

void Settings::setSnapRotation(bool b) {
    if (this->snapRotation == b) {
        return;
    }

    this->snapRotation = b;
    save();
}

auto Settings::getSnapRotationTolerance() const -> double { return this->snapRotationTolerance; }

void Settings::setSnapRotationTolerance(double tolerance) {
    this->snapRotationTolerance = tolerance;
    save();
}

auto Settings::isSnapGrid() const -> bool { return this->snapGrid; }

void Settings::setSnapGrid(bool b) {
    if (this->snapGrid == b) {
        return;
    }

    this->snapGrid = b;
    save();
}

void Settings::setSnapGridTolerance(double tolerance) {
    this->snapGridTolerance = tolerance;
    save();
}

auto Settings::getSnapGridTolerance() const -> double { return this->snapGridTolerance; }
auto Settings::getSnapGridSize() const -> double { return this->snapGridSize; };
void Settings::setSnapGridSize(double gridSize) {
    if (this->snapGridSize == gridSize) {
        return;
    }
    this->snapGridSize = gridSize;
    save();
}

auto Settings::getStrokeRecognizerMinSize() const -> double { return this->strokeRecognizerMinSize; };
void Settings::setStrokeRecognizerMinSize(double value) {
    if (this->strokeRecognizerMinSize == value) {
        return;
    }

    this->strokeRecognizerMinSize = value;
    save();
};

auto Settings::getTouchDrawingEnabled() const -> bool { return this->touchDrawing; }

void Settings::setTouchDrawingEnabled(bool b) {
    if (this->touchDrawing == b) {
        return;
    }

    this->touchDrawing = b;
    save();
}

auto Settings::getGtkTouchInertialScrollingEnabled() const -> bool { return this->gtkTouchInertialScrolling; };

void Settings::setGtkTouchInertialScrollingEnabled(bool b) {
    if (this->gtkTouchInertialScrolling == b) {
        return;
    }

    this->gtkTouchInertialScrolling = b;
    save();
}

auto Settings::isPressureGuessingEnabled() const -> bool { return this->pressureGuessing; }
void Settings::setPressureGuessingEnabled(bool b) {
    if (this->pressureGuessing == b) {
        return;
    }

    this->pressureGuessing = b;
    save();
}

double Settings::getMinimumPressure() const { return this->minimumPressure; }
void Settings::setMinimumPressure(double minimumPressure) {
    if (this->minimumPressure == minimumPressure) {
        return;
    }

    this->minimumPressure = minimumPressure;
    save();
}

double Settings::getPressureMultiplier() const { return this->pressureMultiplier; }
void Settings::setPressureMultiplier(double multiplier) {
    if (this->pressureMultiplier == multiplier) {
        return;
    }

    this->pressureMultiplier = multiplier;
    save();
}

auto Settings::getScrollbarHideType() const -> ScrollbarHideType { return this->scrollbarHideType; }

void Settings::setScrollbarHideType(ScrollbarHideType type) {
    if (this->scrollbarHideType == type) {
        return;
    }

    this->scrollbarHideType = type;

    save();
}

auto Settings::isAutoloadMostRecent() const -> bool { return this->autoloadMostRecent; }

void Settings::setAutoloadMostRecent(bool load) {
    if (this->autoloadMostRecent == load) {
        return;
    }
    this->autoloadMostRecent = load;
    save();
}

auto Settings::isAutoloadPdfXoj() const -> bool { return this->autoloadPdfXoj; }

void Settings::setAutoloadPdfXoj(bool load) {
    if (this->autoloadPdfXoj == load) {
        return;
    }
    this->autoloadPdfXoj = load;
    save();
}

auto Settings::getDefaultSaveName() const -> std::u8string const& { return this->defaultSaveName; }

void Settings::setDefaultSaveName(const std::u8string& name) {
    if (this->defaultSaveName == name) {
        return;
    }

    this->defaultSaveName = name;

    save();
}

auto Settings::getDefaultPdfExportName() const -> std::u8string const& { return this->defaultPdfExportName; }

void Settings::setDefaultPdfExportName(const std::u8string& name) {
    if (this->defaultPdfExportName == name) {
        return;
    }

    this->defaultPdfExportName = name;

    save();
}

auto Settings::getPageTemplate() const -> string const& { return this->pageTemplate; }

void Settings::setPageTemplate(const string& pageTemplate) {
    if (this->pageTemplate == pageTemplate) {
        return;
    }

    this->pageTemplate = pageTemplate;

    save();
}

auto Settings::getSizeUnit() const -> string const& { return sizeUnit; }

void Settings::setSizeUnit(const string& sizeUnit) {
    if (this->sizeUnit == sizeUnit) {
        return;
    }

    this->sizeUnit = sizeUnit;

    save();
}

/**
 * Get size index in XOJ_UNITS
 */
auto Settings::getSizeUnitIndex() const -> int {
    string unit = getSizeUnit();

    for (int i = 0; i < XOJ_UNIT_COUNT; i++) {
        if (unit == XOJ_UNITS[i].name) {
            return i;
        }
    }

    return 0;
}

/**
 * Set size index in XOJ_UNITS
 */
void Settings::setSizeUnitIndex(int sizeUnitId) {
    if (sizeUnitId < 0 || sizeUnitId >= XOJ_UNIT_COUNT) {
        sizeUnitId = 0;
    }

    setSizeUnit(XOJ_UNITS[sizeUnitId].name);
}

void Settings::setShowPairedPages(bool showPairedPages) {
    if (this->showPairedPages == showPairedPages) {
        return;
    }

    this->showPairedPages = showPairedPages;
    save();
}

auto Settings::isShowPairedPages() const -> bool { return this->showPairedPages; }

void Settings::setPresentationMode(bool presentationMode) {
    if (this->presentationMode == presentationMode) {
        return;
    }
    if (presentationMode) {
        this->activeViewMode = PresetViewModeIds::VIEW_MODE_PRESENTATION;
    }
    this->presentationMode = presentationMode;
    save();
}

auto Settings::isPresentationMode() const -> bool {
    return this->activeViewMode == PresetViewModeIds::VIEW_MODE_PRESENTATION;
}

void Settings::setPressureSensitivity(bool pressureSensitivity) {
    if (this->pressureSensitivity == pressureSensitivity) {
        return;
    }
    this->pressureSensitivity = pressureSensitivity;

    save();
}

void Settings::setPairsOffset(int numOffset) {
    if (this->numPairsOffset == numOffset) {
        return;
    }

    this->numPairsOffset = numOffset;
    save();
}

auto Settings::getPairsOffset() const -> int { return this->numPairsOffset; }

void Settings::setEmptyLastPageAppend(EmptyLastPageAppendType emptyLastPageAppend) {
    if (this->emptyLastPageAppend == emptyLastPageAppend) {
        return;
    }

    this->emptyLastPageAppend = emptyLastPageAppend;
    save();
}

auto Settings::getEmptyLastPageAppend() const -> EmptyLastPageAppendType { return this->emptyLastPageAppend; }

void Settings::setViewColumns(int numColumns) {
    if (this->numColumns == numColumns) {
        return;
    }

    this->numColumns = numColumns;
    save();
}

auto Settings::getViewColumns() const -> int { return this->numColumns; }


void Settings::setViewRows(int numRows) {
    if (this->numRows == numRows) {
        return;
    }

    this->numRows = numRows;
    save();
}

auto Settings::getViewRows() const -> int { return this->numRows; }

void Settings::setViewFixedRows(bool viewFixedRows) {
    if (this->viewFixedRows == viewFixedRows) {
        return;
    }

    this->viewFixedRows = viewFixedRows;
    save();
}

auto Settings::isViewFixedRows() const -> bool { return this->viewFixedRows; }

void Settings::setViewLayoutVert(bool vert) {
    if (this->layoutVertical == vert) {
        return;
    }

    this->layoutVertical = vert;
    save();
}

auto Settings::getViewLayoutVert() const -> bool { return this->layoutVertical; }

void Settings::setViewLayoutR2L(bool r2l) {
    if (this->layoutRightToLeft == r2l) {
        return;
    }

    this->layoutRightToLeft = r2l;
    save();
}

auto Settings::getViewLayoutR2L() const -> bool { return this->layoutRightToLeft; }

void Settings::setViewLayoutB2T(bool b2t) {
    if (this->layoutBottomToTop == b2t) {
        return;
    }

    this->layoutBottomToTop = b2t;
    save();
}

auto Settings::getViewLayoutB2T() const -> bool { return this->layoutBottomToTop; }

void Settings::setLastSavePath(fs::path p) {
    this->lastSavePath = std::move(p);
    save();
}

auto Settings::getLastSavePath() const -> fs::path const& { return this->lastSavePath; }

void Settings::setLastOpenPath(fs::path p) {
    this->lastOpenPath = std::move(p);
    save();
}

auto Settings::getLastOpenPath() const -> fs::path const& { return this->lastOpenPath; }

void Settings::setLastImagePath(const fs::path& path) {
    if (this->lastImagePath == path) {
        return;
    }
    this->lastImagePath = path;
    save();
}

auto Settings::getLastImagePath() const -> fs::path const& { return this->lastImagePath; }

void Settings::setZoomStep(double zoomStep) {
    if (this->zoomStep == zoomStep) {
        return;
    }
    this->zoomStep = zoomStep;
    save();
}

auto Settings::getZoomStep() const -> double { return this->zoomStep; }

void Settings::setZoomStepScroll(double zoomStepScroll) {
    if (this->zoomStepScroll == zoomStepScroll) {
        return;
    }
    this->zoomStepScroll = zoomStepScroll;
    save();
}

auto Settings::getZoomStepScroll() const -> double { return this->zoomStepScroll; }

void Settings::setEdgePanSpeed(double speed) {
    if (this->edgePanSpeed == speed) {
        return;
    }
    this->edgePanSpeed = speed;
    save();
}

auto Settings::getEdgePanSpeed() const -> double { return this->edgePanSpeed; }

void Settings::setEdgePanMaxMult(double maxMult) {
    if (this->edgePanMaxMult == maxMult) {
        return;
    }
    this->edgePanMaxMult = maxMult;
    save();
}

auto Settings::getEdgePanMaxMult() const -> double { return this->edgePanMaxMult; }

void Settings::setDisplayDpi(int dpi) {
    if (this->displayDpi == dpi) {
        return;
    }
    this->displayDpi = dpi;
    save();
}

auto Settings::getDisplayDpi() const -> int { return this->displayDpi; }

void Settings::setAreStockIconsUsed(bool use) {
    if (this->useStockIcons == use) {
        return;
    }
    this->useStockIcons = use;
    save();
}

auto Settings::areStockIconsUsed() const -> bool { return this->useStockIcons; }

auto Settings::isFullscreen() const -> bool { return this->fullscreenActive; }

auto Settings::isSidebarVisible() const -> bool { return this->showSidebar; }

void Settings::setSidebarVisible(bool visible) {
    if (this->showSidebar == visible) {
        return;
    }
    this->showSidebar = visible;
    save();
}

auto Settings::isToolbarVisible() const -> bool { return this->showToolbar; }

void Settings::setToolbarVisible(bool visible) {
    if (this->showToolbar == visible) {
        return;
    }
    this->showToolbar = visible;
    save();
}

auto Settings::getSidebarWidth() const -> int { return this->sidebarWidth; }

void Settings::setSidebarWidth(int width) {
    width = std::max(width, 50);

    if (this->sidebarWidth == width) {
        return;
    }
    this->sidebarWidth = width;
    save();
}

void Settings::setMainWndSize(int width, int height) {
    this->mainWndWidth = width;
    this->mainWndHeight = height;

    save();
}

auto Settings::getMainWndWidth() const -> int { return this->mainWndWidth; }

auto Settings::getMainWndHeight() const -> int { return this->mainWndHeight; }

auto Settings::isMainWndMaximized() const -> bool { return this->maximized; }

void Settings::setMainWndMaximized(bool max) {
    if (this->maximized == max) {
        return;
    }
    this->maximized = max;
    save();
}

void Settings::setSelectedToolbar(const string& name) {
    if (this->selectedToolbar == name) {
        return;
    }
    this->selectedToolbar = name;
    save();
}

auto Settings::getSelectedToolbar() const -> string const& { return this->selectedToolbar; }

auto Settings::getCustomElement(const string& name) -> SElement& { return this->data[name]; }

void Settings::customSettingsChanged() { save(); }

auto Settings::getButtonConfig(unsigned int id) -> ButtonConfig* {
    if (id >= this->buttonConfig.size()) {
        g_error("Settings::getButtonConfig try to get id=%i out of range!", id);
        return nullptr;
    }
    return this->buttonConfig[id].get();
}

void Settings::setViewMode(ViewModeId mode, ViewMode viewMode) {
    if (this->viewModes[mode] == viewMode) {
        return;
    }
    this->viewModes.at(mode) = viewMode;
    save();
}

auto Settings::getTouchZoomStartThreshold() const -> double { return this->touchZoomStartThreshold; }
void Settings::setTouchZoomStartThreshold(double threshold) {
    if (this->touchZoomStartThreshold == threshold) {
        return;
    }

    this->touchZoomStartThreshold = threshold;
    save();
}


auto Settings::getPDFPageRerenderThreshold() const -> double { return this->pageRerenderThreshold; }
void Settings::setPDFPageRerenderThreshold(double threshold) {
    if (this->pageRerenderThreshold == threshold) {
        return;
    }

    this->pageRerenderThreshold = threshold;
    save();
}

auto Settings::getPdfPageCacheSize() const -> int { return this->pdfPageCacheSize; }

void Settings::setPdfPageCacheSize(int size) {
    if (this->pdfPageCacheSize == size) {
        return;
    }
    this->pdfPageCacheSize = size;
    save();
}

auto Settings::getPreloadPagesBefore() const -> unsigned int { return this->preloadPagesBefore; }

void Settings::setPreloadPagesBefore(unsigned int n) {
    if (this->preloadPagesBefore == n) {
        return;
    }
    this->preloadPagesBefore = n;
    save();
}

auto Settings::getPreloadPagesAfter() const -> unsigned int { return this->preloadPagesAfter; }

void Settings::setPreloadPagesAfter(unsigned int n) {
    if (this->preloadPagesAfter == n) {
        return;
    }
    this->preloadPagesAfter = n;
    save();
}

auto Settings::isEagerPageCleanup() const -> bool { return this->eagerPageCleanup; }

void Settings::setEagerPageCleanup(bool b) {
    if (this->eagerPageCleanup == b) {
        return;
    }
    this->eagerPageCleanup = b;
    save();
}

auto Settings::getBorderColor() const -> Color { return this->selectionBorderColor; }

void Settings::setBorderColor(Color color) {
    if (this->selectionBorderColor == color) {
        return;
    }
    this->selectionBorderColor = color;
    save();
}

auto Settings::getSelectionColor() const -> Color { return this->selectionMarkerColor; }

void Settings::setSelectionColor(Color color) {
    if (this->selectionMarkerColor == color) {
        return;
    }
    this->selectionMarkerColor = color;
    save();
}

auto Settings::getActiveSelectionColor() const -> Color { return this->activeSelectionColor; }

void Settings::setActiveSelectionColor(Color color) {
    if (this->activeSelectionColor == color) {
        return;
    }
    this->activeSelectionColor = color;
    save();
}

auto Settings::getRecolorParameters() const -> const RecolorParameters& { return this->recolorParameters; }

void Settings::setRecolorParameters(RecolorParameters&& recolor) {
    if (this->recolorParameters == recolor) {
        return;
    }
    this->recolorParameters = recolor;
    save();
}

auto Settings::getBackgroundColor() const -> Color { return this->backgroundColor; }

void Settings::setBackgroundColor(Color color) {
    if (this->backgroundColor == color) {
        return;
    }
    this->backgroundColor = color;
    save();
}

auto Settings::getFont() -> XojFont& { return this->font; }

void Settings::setFont(const XojFont& font) {
    this->font = font;
    save();
}

#ifdef ENABLE_AUDIO
auto Settings::getAudioFolder() const -> fs::path const& { return this->audioFolder; }

void Settings::setAudioFolder(fs::path audioFolder) {
    if (this->audioFolder == audioFolder) {
        return;
    }

    this->audioFolder = std::move(audioFolder);

    save();
}

auto Settings::getAudioInputDevice() const -> PaDeviceIndex { return this->audioInputDevice; }

void Settings::setAudioInputDevice(PaDeviceIndex deviceIndex) {
    if (this->audioInputDevice == deviceIndex) {
        return;
    }
    this->audioInputDevice = deviceIndex;
    save();
}

auto Settings::getAudioOutputDevice() const -> PaDeviceIndex { return this->audioOutputDevice; }

void Settings::setAudioOutputDevice(PaDeviceIndex deviceIndex) {
    if (this->audioOutputDevice == deviceIndex) {
        return;
    }
    this->audioOutputDevice = deviceIndex;
    save();
}

auto Settings::getAudioSampleRate() const -> double { return this->audioSampleRate; }

void Settings::setAudioSampleRate(double sampleRate) {
    if (this->audioSampleRate == sampleRate) {
        return;
    }
    this->audioSampleRate = sampleRate;
    save();
}

auto Settings::getAudioGain() const -> double { return this->audioGain; }

void Settings::setAudioGain(double gain) {
    if (this->audioGain == gain) {
        return;
    }
    this->audioGain = gain;
    save();
}

auto Settings::getDefaultSeekTime() const -> unsigned int { return this->defaultSeekTime; }

void Settings::setDefaultSeekTime(unsigned int t) {
    if (this->defaultSeekTime == t) {
        return;
    }
    this->defaultSeekTime = t;
    save();
}
#endif

auto Settings::getPluginEnabled() const -> string const& { return this->pluginEnabled; }

void Settings::setPluginEnabled(const string& pluginEnabled) {
    if (this->pluginEnabled == pluginEnabled) {
        return;
    }
    this->pluginEnabled = pluginEnabled;
    save();
}

auto Settings::getPluginDisabled() const -> string const& { return this->pluginDisabled; }

void Settings::setPluginDisabled(const string& pluginDisabled) {
    if (this->pluginDisabled == pluginDisabled) {
        return;
    }
    this->pluginDisabled = pluginDisabled;
    save();
}


void Settings::getStrokeFilter(int* ignoreTime, double* ignoreLength, int* successiveTime) const {
    *ignoreTime = this->strokeFilterIgnoreTime;
    *ignoreLength = this->strokeFilterIgnoreLength;
    *successiveTime = this->strokeFilterSuccessiveTime;
}

void Settings::setStrokeFilter(int ignoreTime, double ignoreLength, int successiveTime) {
    this->strokeFilterIgnoreTime = ignoreTime;
    this->strokeFilterIgnoreLength = ignoreLength;
    this->strokeFilterSuccessiveTime = successiveTime;
}

void Settings::setStrokeFilterEnabled(bool enabled) { this->strokeFilterEnabled = enabled; }

auto Settings::getStrokeFilterEnabled() const -> bool { return this->strokeFilterEnabled; }

void Settings::setDoActionOnStrokeFiltered(bool enabled) { this->doActionOnStrokeFiltered = enabled; }

auto Settings::getDoActionOnStrokeFiltered() const -> bool { return this->doActionOnStrokeFiltered; }

void Settings::setTrySelectOnStrokeFiltered(bool enabled) { this->trySelectOnStrokeFiltered = enabled; }

auto Settings::getTrySelectOnStrokeFiltered() const -> bool { return this->trySelectOnStrokeFiltered; }

void Settings::setSnapRecognizedShapesEnabled(bool enabled) { this->snapRecognizedShapesEnabled = enabled; }

auto Settings::getSnapRecognizedShapesEnabled() const -> bool { return this->snapRecognizedShapesEnabled; }


void Settings::setRestoreLineWidthEnabled(bool enabled) { this->restoreLineWidthEnabled = enabled; }

auto Settings::getRestoreLineWidthEnabled() const -> bool { return this->restoreLineWidthEnabled; }

auto Settings::setPreferredLocale(std::string const& locale) -> void { this->preferredLocale = locale; }

auto Settings::getPreferredLocale() const -> std::string { return this->preferredLocale; }

void Settings::setIgnoredStylusEvents(int numEvents) {
    if (this->numIgnoredStylusEvents == numEvents) {
        return;
    }
    this->numIgnoredStylusEvents = std::max<int>(numEvents, 0);
    save();
}

auto Settings::getIgnoredStylusEvents() const -> int { return this->numIgnoredStylusEvents; }

void Settings::setInputSystemTPCButtonEnabled(bool tpcButtonEnabled) {
    if (this->inputSystemTPCButton == tpcButtonEnabled) {
        return;
    }
    this->inputSystemTPCButton = tpcButtonEnabled;
    save();
}

auto Settings::getInputSystemTPCButtonEnabled() const -> bool { return this->inputSystemTPCButton; }

void Settings::setInputSystemDrawOutsideWindowEnabled(bool drawOutsideWindowEnabled) {
    if (this->inputSystemDrawOutsideWindow == drawOutsideWindowEnabled) {
        return;
    }
    this->inputSystemDrawOutsideWindow = drawOutsideWindowEnabled;
    save();
}

auto Settings::getInputSystemDrawOutsideWindowEnabled() const -> bool { return this->inputSystemDrawOutsideWindow; }

void Settings::setDeviceClassForDevice(GdkDevice* device, InputDeviceTypeOption deviceClass) {
    this->setDeviceClassForDevice(gdk_device_get_name(device), gdk_device_get_source(device), deviceClass);
}

void Settings::setDeviceClassForDevice(const string& deviceName, GdkInputSource deviceSource,
                                       InputDeviceTypeOption deviceClass) {
    auto it = inputDeviceClasses.find(deviceName);
    if (it != inputDeviceClasses.end()) {
        it->second.first = deviceClass;
        it->second.second = deviceSource;
    } else {
        inputDeviceClasses.emplace(deviceName, std::make_pair(deviceClass, deviceSource));
    }
}

auto Settings::getKnownInputDevices() const -> std::vector<InputDevice> {
    std::vector<InputDevice> inputDevices;
    for (auto pair: inputDeviceClasses) {
        const std::string& name = pair.first;
        GdkInputSource& source = pair.second.second;
        inputDevices.emplace_back(name, source);
    }
    return inputDevices;
}

auto Settings::getDeviceClassForDevice(GdkDevice* device) const -> InputDeviceTypeOption {
    return this->getDeviceClassForDevice(gdk_device_get_name(device), gdk_device_get_source(device));
}

auto Settings::getDeviceClassForDevice(const string& deviceName, GdkInputSource deviceSource) const
        -> InputDeviceTypeOption {
    auto search = inputDeviceClasses.find(deviceName);
    if (search != inputDeviceClasses.end()) {
        return search->second.first;
    }


    InputDeviceTypeOption deviceType = InputDeviceTypeOption::Disabled;
    switch (deviceSource) {
        case GDK_SOURCE_CURSOR:
#if (GDK_MAJOR_VERSION >= 3 && GDK_MINOR_VERSION >= 22)
        case GDK_SOURCE_TABLET_PAD:
#endif
        case GDK_SOURCE_KEYBOARD:
            deviceType = InputDeviceTypeOption::Disabled;
            break;
        case GDK_SOURCE_MOUSE:
        case GDK_SOURCE_TOUCHPAD:
#if (GDK_MAJOR_VERSION >= 3 && GDK_MINOR_VERSION >= 22)
        case GDK_SOURCE_TRACKPOINT:
#endif
            deviceType = InputDeviceTypeOption::Mouse;
            break;
        case GDK_SOURCE_PEN:
            deviceType = InputDeviceTypeOption::Pen;
            break;
        case GDK_SOURCE_ERASER:
            deviceType = InputDeviceTypeOption::Eraser;
            break;
        case GDK_SOURCE_TOUCHSCREEN:
            deviceType = InputDeviceTypeOption::Touchscreen;
            break;
        default:
            deviceType = InputDeviceTypeOption::Disabled;
    }
    return deviceType;
}

auto Settings::isScrollbarFadeoutDisabled() const -> bool { return disableScrollbarFadeout; }

void Settings::setScrollbarFadeoutDisabled(bool disable) {
    if (disableScrollbarFadeout == disable) {
        return;
    }
    disableScrollbarFadeout = disable;
    save();
}

auto Settings::isAudioDisabled() const -> bool { return disableAudio; }

void Settings::setAudioDisabled(bool disable) {
    if (disableAudio == disable) {
        return;
    }
    disableAudio = disable;
    save();
}

//////////////////////////////////////////////////

SAttribute::SAttribute() {
    this->dValue = 0;
    this->iValue = 0;
    this->type = ATTRIBUTE_TYPE_NONE;
}

SAttribute::SAttribute(const SAttribute& attrib) { *this = attrib; }

SAttribute::~SAttribute() {
    this->iValue = 0;
    this->type = ATTRIBUTE_TYPE_NONE;
}

//////////////////////////////////////////////////

auto SElement::attributes() -> std::map<string, SAttribute>& { return this->element->attributes; }

auto SElement::children() -> std::map<string, SElement>& { return this->element->children; }

void SElement::clear() {
    this->element->attributes.clear();
    this->element->children.clear();
}

auto SElement::child(const string& name) -> SElement& { return this->element->children[name]; }

void SElement::setComment(const string& name, const string& comment) {
    SAttribute& attrib = this->element->attributes[name];
    attrib.comment = comment;
}

template <typename T>
void SElement::set(const string& name, const T value) {
    SAttribute& attrib = this->element->attributes[name];

    if constexpr (std::is_same_v<T, std::string>) {
        attrib.type = ATTRIBUTE_TYPE_STRING;
        attrib.sValue = value;
    } else if constexpr (std::is_same_v<T, int>) {
        attrib.type = ATTRIBUTE_TYPE_INT;
        attrib.iValue = value;
    } else if constexpr (std::is_same_v<T, uint32_t>) {
        attrib.type = ATTRIBUTE_TYPE_INT_HEX;
        attrib.iValue = static_cast<int32_t>(value);
    } else if constexpr (std::is_same_v<T, double>) {
        attrib.type = ATTRIBUTE_TYPE_DOUBLE;
        attrib.dValue = value;
    } else if constexpr (std::is_same_v<T, bool>) {
        attrib.type = ATTRIBUTE_TYPE_BOOLEAN;
        attrib.iValue = value;
    }
}

auto SElement::getDouble(const string& name, double& value) -> bool {
    SAttribute& attrib = this->element->attributes[name];
    if (attrib.type == ATTRIBUTE_TYPE_NONE) {
        this->element->attributes.erase(name);
        return false;
    }

    if (attrib.type != ATTRIBUTE_TYPE_DOUBLE) {
        return false;
    }

    value = attrib.dValue;

    return true;
}

auto SElement::getInt(const string& name, int& value) -> bool {
    SAttribute& attrib = this->element->attributes[name];
    if (attrib.type == ATTRIBUTE_TYPE_NONE) {
        this->element->attributes.erase(name);
        return false;
    }

    if (attrib.type != ATTRIBUTE_TYPE_INT && attrib.type != ATTRIBUTE_TYPE_INT_HEX) {
        return false;
    }

    value = static_cast<int>(attrib.iValue);

    return true;
}

auto SElement::getBool(const string& name, bool& value) -> bool {
    SAttribute& attrib = this->element->attributes[name];
    if (attrib.type == ATTRIBUTE_TYPE_NONE) {
        this->element->attributes.erase(name);
        return false;
    }

    if (attrib.type != ATTRIBUTE_TYPE_BOOLEAN) {
        return false;
    }

    value = attrib.iValue;

    return true;
}

auto SElement::getString(const string& name, string& value) -> bool {
    SAttribute& attrib = this->element->attributes[name];
    if (attrib.type == ATTRIBUTE_TYPE_NONE) {
        this->element->attributes.erase(name);
        return false;
    }

    if (attrib.type != ATTRIBUTE_TYPE_STRING) {
        return false;
    }

    value = attrib.sValue;

    return true;
}

/**
 * Stabilizer related getters and setters
 */
auto Settings::getStabilizerCuspDetection() const -> bool { return stabilizerCuspDetection; }
auto Settings::getStabilizerFinalizeStroke() const -> bool { return stabilizerFinalizeStroke; }
auto Settings::getStabilizerBuffersize() const -> size_t { return stabilizerBuffersize; }
auto Settings::getStabilizerDeadzoneRadius() const -> double { return stabilizerDeadzoneRadius; }
auto Settings::getStabilizerDrag() const -> double { return stabilizerDrag; }
auto Settings::getStabilizerMass() const -> double { return stabilizerMass; }
auto Settings::getStabilizerSigma() const -> double { return stabilizerSigma; }
auto Settings::getStabilizerAveragingMethod() const -> StrokeStabilizer::AveragingMethod {
    return stabilizerAveragingMethod;
}
auto Settings::getStabilizerPreprocessor() const -> StrokeStabilizer::Preprocessor { return stabilizerPreprocessor; }

void Settings::setStabilizerCuspDetection(bool cuspDetection) {
    if (stabilizerCuspDetection == cuspDetection) {
        return;
    }
    stabilizerCuspDetection = cuspDetection;
    save();
}
void Settings::setStabilizerFinalizeStroke(bool finalizeStroke) {
    if (stabilizerFinalizeStroke == finalizeStroke) {
        return;
    }
    stabilizerFinalizeStroke = finalizeStroke;
    save();
}
void Settings::setStabilizerBuffersize(size_t buffersize) {
    if (stabilizerBuffersize == buffersize) {
        return;
    }
    stabilizerBuffersize = buffersize;
    save();
}
void Settings::setStabilizerDeadzoneRadius(double deadzoneRadius) {
    if (stabilizerDeadzoneRadius == deadzoneRadius) {
        return;
    }
    stabilizerDeadzoneRadius = deadzoneRadius;
    save();
}
void Settings::setStabilizerDrag(double drag) {
    if (stabilizerDrag == drag) {
        return;
    }
    stabilizerDrag = drag;
    save();
}
void Settings::setStabilizerMass(double mass) {
    if (stabilizerMass == mass) {
        return;
    }
    stabilizerMass = mass;
    save();
}
void Settings::setStabilizerSigma(double sigma) {
    if (stabilizerSigma == sigma) {
        return;
    }
    stabilizerSigma = sigma;
    save();
}
void Settings::setStabilizerAveragingMethod(StrokeStabilizer::AveragingMethod averagingMethod) {
    const StrokeStabilizer::AveragingMethod method =
            StrokeStabilizer::isValid(averagingMethod) ? averagingMethod : StrokeStabilizer::AveragingMethod::NONE;

    if (stabilizerAveragingMethod == method) {
        return;
    }
    stabilizerAveragingMethod = method;
    save();
}
void Settings::setStabilizerPreprocessor(StrokeStabilizer::Preprocessor preprocessor) {
    const StrokeStabilizer::Preprocessor p =
            StrokeStabilizer::isValid(preprocessor) ? preprocessor : StrokeStabilizer::Preprocessor::NONE;

    if (stabilizerPreprocessor == p) {
        return;
    }
    stabilizerPreprocessor = p;
    save();
}


auto Settings::getColorPaletteSetting() -> fs::path const& { return this->colorPaletteSetting; }

void Settings::setColorPaletteSetting(fs::path palettePath) { this->colorPaletteSetting = palettePath; }


void Settings::setUseSpacesAsTab(bool useSpaces) { this->useSpacesForTab = useSpaces; }
bool Settings::getUseSpacesAsTab() const { return this->useSpacesForTab; }

void Settings::setNumberOfSpacesForTab(unsigned int numberOfSpaces) {
    if (this->numberOfSpacesForTab == numberOfSpaces) {
        return;
    }

    // For performance reasons the number of spaces for a tab should be limited
    // if this limit is exceeded use a default value
    if (numberOfSpaces < 0 || numberOfSpaces > MAX_SPACES_FOR_TAB) {
        g_warning("Settings::Invalid number of spaces for tab. Reset to default!");
        numberOfSpaces = 4;
    }
    this->numberOfSpacesForTab = numberOfSpaces;
    save();
}

unsigned int Settings::getNumberOfSpacesForTab() const { return this->numberOfSpacesForTab; }

void Settings::setLaserPointerFadeOutTime(unsigned int timeInMs) {
    if (this->laserPointerFadeOutTime == timeInMs) {
        return;
    }
    this->laserPointerFadeOutTime = timeInMs;
    save();
}

unsigned int Settings::getLaserPointerFadeOutTime() const { return this->laserPointerFadeOutTime; }
