#include "Settings.h"

#include <algorithm>    // for max
#include <cstdint>      // for uint32_t, int32_t
#include <cstdio>       // for sscanf, size_t
#include <cstdlib>      // for atoi
#include <cstring>      // for strcmp
#include <exception>    // for exception
#include <type_traits>  // for add_const<>::type
#include <utility>      // for pair, move, make_...

#include <libxml/globals.h>    // for xmlFree, xmlInden...
#include <libxml/parser.h>     // for xmlKeepBlanksDefault
#include <libxml/xmlstring.h>  // for xmlStrcmp, xmlChar

#include "control/DeviceListHelper.h"               // for InputDevice
#include "control/ToolEnums.h"                      // for ERASER_TYPE_NONE
#include "control/settings/LatexSettings.h"         // for LatexSettings
#include "control/settings/SettingsEnums.h"         // for InputDeviceTypeOp...
#include "gui/toolbarMenubar/model/ColorPalette.h"  // for Palette
#include "model/FormatDefinitions.h"                // for FormatUnits, XOJ_...
#include "util/Color.h"
#include "util/PathUtil.h"  // for getConfigFile
#include "util/Util.h"      // for PRECISION_FORMAT_...
#include "util/i18n.h"      // for _

#include "ButtonConfig.h"  // for ButtonConfig
#include "config-dev.h"    // for PALETTE_FILE
#include "filesystem.h"    // for path, u8path, exists


using std::string;

constexpr auto const* DEFAULT_FONT = "Sans";
constexpr auto DEFAULT_FONT_SIZE = 12;

#define SAVE_BOOL_PROP(var) xmlNode = saveProperty((const char*)#var, (var) ? "true" : "false", root)
#define SAVE_STRING_PROP(var) xmlNode = saveProperty((const char*)#var, (var).empty() ? "" : (var).c_str(), root)
#define SAVE_FONT_PROP(var) xmlNode = saveProperty((const char*)#var, var.asString().c_str(), root)
#define SAVE_INT_PROP(var) xmlNode = saveProperty((const char*)#var, var, root)
#define SAVE_UINT_PROP(var) xmlNode = savePropertyUnsigned((const char*)#var, var, root)
#define SAVE_DOUBLE_PROP(var) xmlNode = savePropertyDouble((const char*)#var, var, root)
#define ATTACH_COMMENT(var)                     \
    com = xmlNewComment((const xmlChar*)(var)); \
    xmlAddPrevSibling(xmlNode, com);

Settings::Settings(fs::path filepath): filepath(std::move(filepath)) { loadDefault(); }

Settings::~Settings() = default;

void Settings::loadDefault() {
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

    this->displayDpi = 72;

    this->font.setName(DEFAULT_FONT);
    this->font.setSize(DEFAULT_FONT_SIZE);

    this->mainWndWidth = 800;
    this->mainWndHeight = 600;

    this->fullscreenActive = false;

    this->showSidebar = true;
    this->sidebarWidth = 150;
    this->sidebarNumberingStyle = SidebarNumberingStyle::DEFAULT;

    this->showToolbar = true;

    this->sidebarOnRight = false;

    this->scrollbarOnLeft = false;

    this->menubarVisible = true;

    this->autoloadMostRecent = false;
    this->autoloadPdfXoj = true;

    this->stylusCursorType = STYLUS_CURSOR_DOT;
    this->eraserVisibility = ERASER_VISIBILITY_ALWAYS;
    this->iconTheme = ICON_THEME_COLOR;
    this->highlightPosition = false;
    this->cursorHighlightColor = 0x80FFFF00;  // Yellow with 50% opacity
    this->cursorHighlightRadius = 30.0;
    this->cursorHighlightBorderColor = 0x800000FF;  // Blue with 50% opacity
    this->cursorHighlightBorderWidth = 0.0;
    this->darkTheme = false;
    this->useStockIcons = false;
    this->scrollbarHideType = SCROLLBAR_HIDE_NONE;
    this->disableScrollbarFadeout = false;

    // Set this for autosave frequency in minutes.
    this->autosaveTimeout = 3;
    this->autosaveEnabled = true;

    this->addHorizontalSpace = false;
    this->addHorizontalSpaceAmount = 150;
    this->addVerticalSpace = false;
    this->addVerticalSpaceAmount = 150;

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

    this->defaultSaveName = _("%F-Note-%H-%M");

    this->defaultPdfExportName = _("%{name}_annotated");

    // Eraser
    this->buttonConfig[BUTTON_ERASER] = std::make_unique<ButtonConfig>(TOOL_ERASER, Colors::black, TOOL_SIZE_NONE,
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

    this->backgroundColor = Colors::xopp_gainsboro02;

    // clang-format off
	this->pageTemplate = "xoj/template\ncopyLastPageSettings=true\nsize=595.275591x841.889764\nbackgroundType=lined\nbackgroundColor=#ffffff\n";
    // clang-format on

    this->audioSampleRate = 44100.0;
    this->audioInputDevice = AUDIO_INPUT_SYSTEM_DEFAULT;
    this->audioOutputDevice = AUDIO_OUTPUT_SYSTEM_DEFAULT;
    this->audioGain = 1.0;
    this->defaultSeekTime = 5;

    this->pluginEnabled = "";
    this->pluginDisabled = "";

    this->numIgnoredStylusEvents = 0;

    this->inputSystemTPCButton = false;
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

/**
 * tempg_ascii_strtod
 * 	Transition to using g_ascii_strtod to minimize disruption. May, 2019.
 *  Delete this and replace calls to this function with calls to g_ascii_strtod() in 2020.
 * 	See: https://developer.gnome.org/glib/stable/glib-String-Utility-Functions.html#g-strtod
 */
auto tempg_ascii_strtod(const gchar* txt, gchar** endptr) -> double {
    return g_strtod(txt,
                    endptr);  //  makes best guess between locale formatted and C formatted numbers. See link above.
}


void Settings::parseData(xmlNodePtr cur, SElement& elem) {
    for (xmlNodePtr x = cur->children; x != nullptr; x = x->next) {
        if (!xmlStrcmp(x->name, reinterpret_cast<const xmlChar*>("data"))) {
            xmlChar* name = xmlGetProp(x, reinterpret_cast<const xmlChar*>("name"));
            parseData(x, elem.child(reinterpret_cast<const char*>(name)));
            xmlFree(name);
        } else if (!xmlStrcmp(x->name, reinterpret_cast<const xmlChar*>("attribute"))) {
            xmlChar* name = xmlGetProp(x, reinterpret_cast<const xmlChar*>("name"));
            xmlChar* value = xmlGetProp(x, reinterpret_cast<const xmlChar*>("value"));
            xmlChar* type = xmlGetProp(x, reinterpret_cast<const xmlChar*>("type"));

            string sType = reinterpret_cast<const char*>(type);

            if (sType == "int") {
                int i = atoi(reinterpret_cast<const char*>(value));
                elem.setInt(reinterpret_cast<const char*>(name), i);
            } else if (sType == "double") {
                double d = tempg_ascii_strtod(reinterpret_cast<const char*>(value),
                                              nullptr);  // g_ascii_strtod ignores locale setting.
                elem.setDouble(reinterpret_cast<const char*>(name), d);
            } else if (sType == "hex") {
                int i = 0;
                if (sscanf(reinterpret_cast<const char*>(value), "%x", &i)) {
                    elem.setIntHex(reinterpret_cast<const char*>(name), i);
                } else {
                    g_warning("Settings::Unknown hex value: %s:%s\n", name, value);
                }
            } else if (sType == "string") {
                elem.setString(reinterpret_cast<const char*>(name), reinterpret_cast<const char*>(value));
            } else if (sType == "boolean") {
                elem.setBool(reinterpret_cast<const char*>(name),
                             strcmp(reinterpret_cast<const char*>(value), "true") == 0);
            } else {
                g_warning("Settings::Unknown datatype: %s\n", sType.c_str());
            }

            xmlFree(name);
            xmlFree(type);
            xmlFree(value);
        } else {
            g_warning("Settings::parseData: Unknown XML node: %s\n", x->name);
            continue;
        }
    }
}

void Settings::parseItem(xmlDocPtr doc, xmlNodePtr cur) {
    // Parse data map
    if (!xmlStrcmp(cur->name, reinterpret_cast<const xmlChar*>("data"))) {
        xmlChar* name = xmlGetProp(cur, reinterpret_cast<const xmlChar*>("name"));
        if (name == nullptr) {
            g_warning("Settings::%s:No name property!\n", cur->name);
            return;
        }

        parseData(cur, data[reinterpret_cast<const char*>(name)]);

        xmlFree(name);
        return;
    }

    if (cur->type == XML_COMMENT_NODE) {
        return;
    }

    if (xmlStrcmp(cur->name, reinterpret_cast<const xmlChar*>("property"))) {
        g_warning("Settings::Unknown XML node: %s\n", cur->name);
        return;
    }

    xmlChar* name = xmlGetProp(cur, reinterpret_cast<const xmlChar*>("name"));
    if (name == nullptr) {
        g_warning("Settings::%s:No name property!\n", cur->name);
        return;
    }

    if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("font")) == 0) {
        xmlFree(name);
        xmlChar* font = nullptr;
        xmlChar* size = nullptr;

        font = xmlGetProp(cur, reinterpret_cast<const xmlChar*>("font"));
        if (font) {
            this->font.setName(reinterpret_cast<const char*>(font));
            xmlFree(font);
        }

        size = xmlGetProp(cur, reinterpret_cast<const xmlChar*>("size"));
        if (size) {
            double dSize = DEFAULT_FONT_SIZE;
            if (sscanf(reinterpret_cast<const char*>(size), "%lf", &dSize) == 1) {
                this->font.setSize(dSize);
            }
            xmlFree(size);
        }
        return;
    }

    xmlChar* value = xmlGetProp(cur, reinterpret_cast<const xmlChar*>("value"));
    if (value == nullptr) {
        xmlFree(name);
        g_warning("Settings::No value property!\n");
        return;
    }

    // TODO(fabian): remove this typo fix in 2-3 release cycles
    if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("presureSensitivity")) == 0) {
        this->pressureSensitivity = xmlStrcmp(value, reinterpret_cast<const xmlChar*>("true")) == 0;
    }
    if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("pressureSensitivity")) == 0) {
        this->pressureSensitivity = xmlStrcmp(value, reinterpret_cast<const xmlChar*>("true")) == 0;
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("minimumPressure")) == 0) {
        this->minimumPressure = g_ascii_strtod(reinterpret_cast<const char*>(value), nullptr);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("pressureMultiplier")) == 0) {
        this->pressureMultiplier = g_ascii_strtod(reinterpret_cast<const char*>(value), nullptr);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("zoomGesturesEnabled")) == 0) {
        this->zoomGesturesEnabled = xmlStrcmp(value, reinterpret_cast<const xmlChar*>("true")) == 0;
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("selectedToolbar")) == 0) {
        this->selectedToolbar = reinterpret_cast<const char*>(value);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("lastSavePath")) == 0) {
        this->lastSavePath = fs::u8path(reinterpret_cast<const char*>(value));
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("lastOpenPath")) == 0) {
        this->lastOpenPath = fs::u8path(reinterpret_cast<const char*>(value));
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("lastImagePath")) == 0) {
        this->lastImagePath = fs::u8path(reinterpret_cast<const char*>(value));
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("edgePanSpeed")) == 0) {
        this->edgePanSpeed = tempg_ascii_strtod(reinterpret_cast<const char*>(value), nullptr);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("edgePanMaxMult")) == 0) {
        this->edgePanMaxMult = tempg_ascii_strtod(reinterpret_cast<const char*>(value), nullptr);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("zoomStep")) == 0) {
        this->zoomStep = tempg_ascii_strtod(reinterpret_cast<const char*>(value), nullptr);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("zoomStepScroll")) == 0) {
        this->zoomStepScroll = tempg_ascii_strtod(reinterpret_cast<const char*>(value), nullptr);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("displayDpi")) == 0) {
        this->displayDpi = g_ascii_strtoll(reinterpret_cast<const char*>(value), nullptr, 10);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("mainWndWidth")) == 0) {
        this->mainWndWidth = g_ascii_strtoll(reinterpret_cast<const char*>(value), nullptr, 10);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("mainWndHeight")) == 0) {
        this->mainWndHeight = g_ascii_strtoll(reinterpret_cast<const char*>(value), nullptr, 10);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("maximized")) == 0) {
        this->maximized = xmlStrcmp(value, reinterpret_cast<const xmlChar*>("true")) == 0;
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("showToolbar")) == 0) {
        this->showToolbar = xmlStrcmp(value, reinterpret_cast<const xmlChar*>("true")) == 0;
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("filepathShownInTitlebar")) == 0) {
        this->filepathShownInTitlebar = xmlStrcmp(value, reinterpret_cast<const xmlChar*>("true")) == 0;
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("showSidebar")) == 0) {
        this->showSidebar = xmlStrcmp(value, reinterpret_cast<const xmlChar*>("true")) == 0;
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("sidebarNumberingStyle")) == 0) {
        int num = std::stoi(reinterpret_cast<char*>(value));
        if (num < static_cast<int>(SidebarNumberingStyle::MIN) || static_cast<int>(SidebarNumberingStyle::MAX) < num) {
            num = static_cast<int>(SidebarNumberingStyle::DEFAULT);
            g_warning("Settings::Invalid sidebarNumberingStyle value. Reset to default.");
        }
        this->sidebarNumberingStyle = static_cast<SidebarNumberingStyle>(num);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("sidebarWidth")) == 0) {
        this->sidebarWidth = std::max<int>(g_ascii_strtoll(reinterpret_cast<const char*>(value), nullptr, 10), 50);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("sidebarOnRight")) == 0) {
        this->sidebarOnRight = xmlStrcmp(value, reinterpret_cast<const xmlChar*>("true")) == 0;
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("scrollbarOnLeft")) == 0) {
        this->scrollbarOnLeft = xmlStrcmp(value, reinterpret_cast<const xmlChar*>("true")) == 0;
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("menubarVisible")) == 0) {
        this->menubarVisible = xmlStrcmp(value, reinterpret_cast<const xmlChar*>("true")) == 0;
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("numColumns")) == 0) {
        this->numColumns = g_ascii_strtoll(reinterpret_cast<const char*>(value), nullptr, 10);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("numRows")) == 0) {
        this->numRows = g_ascii_strtoll(reinterpret_cast<const char*>(value), nullptr, 10);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("viewFixedRows")) == 0) {
        this->viewFixedRows = xmlStrcmp(value, reinterpret_cast<const xmlChar*>("true")) == 0;
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("layoutVertical")) == 0) {
        this->layoutVertical = xmlStrcmp(value, reinterpret_cast<const xmlChar*>("true")) == 0;
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("layoutRightToLeft")) == 0) {
        this->layoutRightToLeft = xmlStrcmp(value, reinterpret_cast<const xmlChar*>("true")) == 0;
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("layoutBottomToTop")) == 0) {
        this->layoutBottomToTop = xmlStrcmp(value, reinterpret_cast<const xmlChar*>("true")) == 0;
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("showPairedPages")) == 0) {
        this->showPairedPages = xmlStrcmp(value, reinterpret_cast<const xmlChar*>("true")) == 0;
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("numPairsOffset")) == 0) {
        this->numPairsOffset = g_ascii_strtoll(reinterpret_cast<const char*>(value), nullptr, 10);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("presentationMode")) == 0) {
        this->presentationMode = xmlStrcmp(value, reinterpret_cast<const xmlChar*>("true")) == 0;
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("autoloadMostRecent")) == 0) {
        this->autoloadMostRecent = xmlStrcmp(value, reinterpret_cast<const xmlChar*>("true")) == 0;
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("autoloadPdfXoj")) == 0) {
        this->autoloadPdfXoj = xmlStrcmp(value, reinterpret_cast<const xmlChar*>("true")) == 0;
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("stylusCursorType")) == 0) {
        this->stylusCursorType = stylusCursorTypeFromString(reinterpret_cast<const char*>(value));
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("eraserVisibility")) == 0) {
        this->eraserVisibility = eraserVisibilityFromString(reinterpret_cast<const char*>(value));
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("iconTheme")) == 0) {
        this->iconTheme = iconThemeFromString(reinterpret_cast<const char*>(value));
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("highlightPosition")) == 0) {
        this->highlightPosition = xmlStrcmp(value, reinterpret_cast<const xmlChar*>("true")) == 0;
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("cursorHighlightColor")) == 0) {
        this->cursorHighlightColor = g_ascii_strtoull(reinterpret_cast<const char*>(value), nullptr, 10);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("cursorHighlightRadius")) == 0) {
        this->cursorHighlightRadius = g_ascii_strtod(reinterpret_cast<const char*>(value), nullptr);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("cursorHighlightBorderColor")) == 0) {
        this->cursorHighlightBorderColor = g_ascii_strtoull(reinterpret_cast<const char*>(value), nullptr, 10);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("cursorHighlightBorderWidth")) == 0) {
        this->cursorHighlightBorderWidth = g_ascii_strtod(reinterpret_cast<const char*>(value), nullptr);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("darkTheme")) == 0) {
        this->darkTheme = xmlStrcmp(value, reinterpret_cast<const xmlChar*>("true")) == 0;
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("useStockIcons")) == 0) {
        this->useStockIcons = xmlStrcmp(value, reinterpret_cast<const xmlChar*>("true")) == 0;
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("defaultSaveName")) == 0) {
        this->defaultSaveName = reinterpret_cast<const char*>(value);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("defaultPdfExportName")) == 0) {
        this->defaultPdfExportName = reinterpret_cast<const char*>(value);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("pluginEnabled")) == 0) {
        this->pluginEnabled = reinterpret_cast<const char*>(value);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("pluginDisabled")) == 0) {
        this->pluginDisabled = reinterpret_cast<const char*>(value);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("pageTemplate")) == 0) {
        this->pageTemplate = reinterpret_cast<const char*>(value);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("sizeUnit")) == 0) {
        this->sizeUnit = reinterpret_cast<const char*>(value);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("audioFolder")) == 0) {
        this->audioFolder = fs::u8path(reinterpret_cast<const char*>(value));
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("autosaveEnabled")) == 0) {
        this->autosaveEnabled = xmlStrcmp(value, reinterpret_cast<const xmlChar*>("true")) == 0;
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("autosaveTimeout")) == 0) {
        this->autosaveTimeout = g_ascii_strtoll(reinterpret_cast<const char*>(value), nullptr, 10);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("defaultViewModeAttributes")) == 0) {
        this->viewModes.at(PresetViewModeIds::VIEW_MODE_DEFAULT) =
                settingsStringToViewMode(reinterpret_cast<const char*>(value));
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("fullscreenViewModeAttributes")) == 0) {
        this->viewModes.at(PresetViewModeIds::VIEW_MODE_FULLSCREEN) =
                settingsStringToViewMode(reinterpret_cast<const char*>(value));
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("presentationViewModeAttributes")) == 0) {
        this->viewModes.at(PresetViewModeIds::VIEW_MODE_PRESENTATION) =
                settingsStringToViewMode(reinterpret_cast<const char*>(value));
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("touchZoomStartThreshold")) == 0) {
        this->touchZoomStartThreshold = g_ascii_strtod(reinterpret_cast<const char*>(value), nullptr);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("pageRerenderThreshold")) == 0) {
        this->pageRerenderThreshold = g_ascii_strtod(reinterpret_cast<const char*>(value), nullptr);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("pdfPageCacheSize")) == 0) {
        this->pdfPageCacheSize = g_ascii_strtoll(reinterpret_cast<const char*>(value), nullptr, 10);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("preloadPagesBefore")) == 0) {
        this->preloadPagesBefore = g_ascii_strtoull(reinterpret_cast<const char*>(value), nullptr, 10);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("preloadPagesAfter")) == 0) {
        this->preloadPagesAfter = g_ascii_strtoull(reinterpret_cast<const char*>(value), nullptr, 10);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("eagerPageCleanup")) == 0) {
        this->eagerPageCleanup = xmlStrcmp(value, reinterpret_cast<const xmlChar*>("true")) == 0;
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("selectionBorderColor")) == 0) {
        this->selectionBorderColor = Color(g_ascii_strtoull(reinterpret_cast<const char*>(value), nullptr, 10));
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("selectionMarkerColor")) == 0) {
        this->selectionMarkerColor = Color(g_ascii_strtoull(reinterpret_cast<const char*>(value), nullptr, 10));
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("backgroundColor")) == 0) {
        this->backgroundColor = Color(g_ascii_strtoull(reinterpret_cast<const char*>(value), nullptr, 10));
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("addHorizontalSpace")) == 0) {
        this->addHorizontalSpace = xmlStrcmp(value, reinterpret_cast<const xmlChar*>("true")) == 0;
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("addHorizontalSpaceAmount")) == 0) {
        this->addHorizontalSpaceAmount = g_ascii_strtoll(reinterpret_cast<const char*>(value), nullptr, 10);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("addVerticalSpace")) == 0) {
        this->addVerticalSpace = xmlStrcmp(value, reinterpret_cast<const xmlChar*>("true")) == 0;
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("addVerticalSpaceAmount")) == 0) {
        this->addVerticalSpaceAmount = g_ascii_strtoll(reinterpret_cast<const char*>(value), nullptr, 10);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("drawDirModsEnabled")) == 0) {
        this->drawDirModsEnabled = xmlStrcmp(value, reinterpret_cast<const xmlChar*>("true")) == 0;
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("drawDirModsRadius")) == 0) {
        this->drawDirModsRadius = g_ascii_strtoll(reinterpret_cast<const char*>(value), nullptr, 10);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("snapRotation")) == 0) {
        this->snapRotation = xmlStrcmp(value, reinterpret_cast<const xmlChar*>("true")) == 0;
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("snapRotationTolerance")) == 0) {
        this->snapRotationTolerance = tempg_ascii_strtod(reinterpret_cast<const char*>(value), nullptr);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("snapGrid")) == 0) {
        this->snapGrid = xmlStrcmp(value, reinterpret_cast<const xmlChar*>("true")) == 0;
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("snapGridSize")) == 0) {
        this->snapGridSize = tempg_ascii_strtod(reinterpret_cast<const char*>(value), nullptr);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("snapGridTolerance")) == 0) {
        this->snapGridTolerance = tempg_ascii_strtod(reinterpret_cast<const char*>(value), nullptr);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("strokeRecognizerMinSize")) == 0) {
        this->strokeRecognizerMinSize = tempg_ascii_strtod(reinterpret_cast<const char*>(value), nullptr);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("touchDrawing")) == 0) {
        this->touchDrawing = xmlStrcmp(value, reinterpret_cast<const xmlChar*>("true")) == 0;
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("gtkTouchInertialScrolling")) == 0) {
        this->gtkTouchInertialScrolling = xmlStrcmp(value, reinterpret_cast<const xmlChar*>("true")) == 0;
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("pressureGuessing")) == 0) {
        this->pressureGuessing = xmlStrcmp(value, reinterpret_cast<const xmlChar*>("true")) == 0;
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("scrollbarHideType")) == 0) {
        if (xmlStrcmp(value, reinterpret_cast<const xmlChar*>("both")) == 0) {
            this->scrollbarHideType = SCROLLBAR_HIDE_BOTH;
        } else if (xmlStrcmp(value, reinterpret_cast<const xmlChar*>("horizontal")) == 0) {
            this->scrollbarHideType = SCROLLBAR_HIDE_HORIZONTAL;
        } else if (xmlStrcmp(value, reinterpret_cast<const xmlChar*>("vertical")) == 0) {
            this->scrollbarHideType = SCROLLBAR_HIDE_VERTICAL;
        } else {
            this->scrollbarHideType = SCROLLBAR_HIDE_NONE;
        }
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("disableScrollbarFadeout")) == 0) {
        this->disableScrollbarFadeout = xmlStrcmp(value, reinterpret_cast<const xmlChar*>("true")) == 0;
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("audioSampleRate")) == 0) {
        this->audioSampleRate = tempg_ascii_strtod(reinterpret_cast<const char*>(value), nullptr);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("audioGain")) == 0) {
        this->audioGain = tempg_ascii_strtod(reinterpret_cast<const char*>(value), nullptr);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("defaultSeekTime")) == 0) {
        this->defaultSeekTime = tempg_ascii_strtod(reinterpret_cast<const char*>(value), nullptr);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("audioInputDevice")) == 0) {
        this->audioInputDevice = g_ascii_strtoll(reinterpret_cast<const char*>(value), nullptr, 10);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("audioOutputDevice")) == 0) {
        this->audioOutputDevice = g_ascii_strtoll(reinterpret_cast<const char*>(value), nullptr, 10);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("numIgnoredStylusEvents")) == 0) {
        this->numIgnoredStylusEvents =
                std::max<int>(g_ascii_strtoll(reinterpret_cast<const char*>(value), nullptr, 10), 0);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("inputSystemTPCButton")) == 0) {
        this->inputSystemTPCButton = xmlStrcmp(value, reinterpret_cast<const xmlChar*>("true")) == 0;
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("inputSystemDrawOutsideWindow")) == 0) {
        this->inputSystemDrawOutsideWindow = xmlStrcmp(value, reinterpret_cast<const xmlChar*>("true")) == 0;
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("emptyLastPageAppend")) == 0) {
        this->emptyLastPageAppend = emptyLastPageAppendFromString(reinterpret_cast<char*>(value));
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("strokeFilterIgnoreTime")) == 0) {
        this->strokeFilterIgnoreTime = g_ascii_strtoll(reinterpret_cast<const char*>(value), nullptr, 10);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("strokeFilterIgnoreLength")) == 0) {
        this->strokeFilterIgnoreLength = tempg_ascii_strtod(reinterpret_cast<const char*>(value), nullptr);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("strokeFilterSuccessiveTime")) == 0) {
        this->strokeFilterSuccessiveTime = g_ascii_strtoll(reinterpret_cast<const char*>(value), nullptr, 10);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("strokeFilterEnabled")) == 0) {
        this->strokeFilterEnabled = xmlStrcmp(value, reinterpret_cast<const xmlChar*>("true")) == 0;
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("doActionOnStrokeFiltered")) == 0) {
        this->doActionOnStrokeFiltered = xmlStrcmp(value, reinterpret_cast<const xmlChar*>("true")) == 0;
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("trySelectOnStrokeFiltered")) == 0) {
        this->trySelectOnStrokeFiltered = xmlStrcmp(value, reinterpret_cast<const xmlChar*>("true")) == 0;
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("latexSettings.autoCheckDependencies")) == 0) {
        this->latexSettings.autoCheckDependencies = xmlStrcmp(value, reinterpret_cast<const xmlChar*>("true")) == 0;
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("latexSettings.defaultText")) == 0) {
        this->latexSettings.defaultText = reinterpret_cast<char*>(value);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("latexSettings.globalTemplatePath")) == 0) {
        std::string v(reinterpret_cast<char*>(value));
        this->latexSettings.globalTemplatePath = fs::u8path(v);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("latexSettings.genCmd")) == 0) {
        this->latexSettings.genCmd = reinterpret_cast<char*>(value);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("latexSettings.sourceViewThemeId")) == 0) {
        this->latexSettings.sourceViewThemeId = reinterpret_cast<char*>(value);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("latexSettings.editorFont")) == 0) {
        this->latexSettings.editorFont = std::string{reinterpret_cast<char*>(value)};
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("latexSettings.useCustomEditorFont")) == 0) {
        this->latexSettings.useCustomEditorFont = xmlStrcmp(value, reinterpret_cast<const xmlChar*>("true")) == 0;
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("latexSettings.editorWordWrap")) == 0) {
        this->latexSettings.editorWordWrap = xmlStrcmp(value, reinterpret_cast<const xmlChar*>("true")) == 0;
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("latexSettings.sourceViewAutoIndent")) == 0) {
        this->latexSettings.sourceViewAutoIndent = xmlStrcmp(value, reinterpret_cast<const xmlChar*>("true")) == 0;
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("latexSettings.sourceViewSyntaxHighlight")) == 0) {
        this->latexSettings.sourceViewSyntaxHighlight = xmlStrcmp(value, reinterpret_cast<const xmlChar*>("true")) == 0;
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("latexSettings.sourceViewShowLineNumbers")) == 0) {
        this->latexSettings.sourceViewShowLineNumbers = xmlStrcmp(value, reinterpret_cast<const xmlChar*>("true")) == 0;
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("snapRecognizedShapesEnabled")) == 0) {
        this->snapRecognizedShapesEnabled = xmlStrcmp(value, reinterpret_cast<const xmlChar*>("true")) == 0;
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("restoreLineWidthEnabled")) == 0) {
        this->restoreLineWidthEnabled = xmlStrcmp(value, reinterpret_cast<const xmlChar*>("true")) == 0;
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("preferredLocale")) == 0) {
        this->preferredLocale = reinterpret_cast<char*>(value);
        /**
         * Stabilizer related settings
         */
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("stabilizerAveragingMethod")) == 0) {
        this->stabilizerAveragingMethod =
                (StrokeStabilizer::AveragingMethod)g_ascii_strtoll(reinterpret_cast<const char*>(value), nullptr, 10);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("stabilizerPreprocessor")) == 0) {
        this->stabilizerPreprocessor =
                (StrokeStabilizer::Preprocessor)g_ascii_strtoll(reinterpret_cast<const char*>(value), nullptr, 10);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("stabilizerBuffersize")) == 0) {
        this->stabilizerBuffersize = g_ascii_strtoull(reinterpret_cast<const char*>(value), nullptr, 10);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("stabilizerSigma")) == 0) {
        this->stabilizerSigma = tempg_ascii_strtod(reinterpret_cast<const char*>(value), nullptr);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("stabilizerDeadzoneRadius")) == 0) {
        this->stabilizerDeadzoneRadius = tempg_ascii_strtod(reinterpret_cast<const char*>(value), nullptr);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("stabilizerDrag")) == 0) {
        this->stabilizerDrag = tempg_ascii_strtod(reinterpret_cast<const char*>(value), nullptr);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("stabilizerMass")) == 0) {
        this->stabilizerMass = tempg_ascii_strtod(reinterpret_cast<const char*>(value), nullptr);
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("stabilizerCuspDetection")) == 0) {
        this->stabilizerCuspDetection = xmlStrcmp(value, reinterpret_cast<const xmlChar*>("true")) == 0;
    } else if (xmlStrcmp(name, reinterpret_cast<const xmlChar*>("stabilizerFinalizeStroke")) == 0) {
        this->stabilizerFinalizeStroke = xmlStrcmp(value, reinterpret_cast<const xmlChar*>("true")) == 0;
    }
    /**/

    xmlFree(name);
    xmlFree(value);
}

void Settings::loadDeviceClasses() {
    SElement& s = getCustomElement("deviceClasses");
    for (auto device: s.children()) {
        SElement& deviceNode = device.second;
        int deviceClass = 0;
        int deviceSource = 0;
        deviceNode.getInt("deviceClass", deviceClass);
        deviceNode.getInt("deviceSource", deviceSource);
        inputDeviceClasses.emplace(device.first, std::make_pair(static_cast<InputDeviceTypeOption>(deviceClass),
                                                                static_cast<GdkInputSource>(deviceSource)));
    }
}

void Settings::loadButtonConfig() {
    SElement& s = getCustomElement("buttonConfig");

    for (int i = 0; i < BUTTON_COUNT; i++) {
        SElement& e = s.child(buttonToString(static_cast<Button>(i)));
        const auto& cfg = buttonConfig[i];

        string sType;
        if (e.getString("tool", sType)) {
            ToolType type = toolTypeFromString(sType);
            cfg->action = type;

            if (type == TOOL_PEN || type == TOOL_HIGHLIGHTER) {
                string drawingType;
                if (e.getString("drawingType", drawingType)) {
                    cfg->drawingType = drawingTypeFromString(drawingType);
                }

                string sSize;
                if (e.getString("size", sSize)) {
                    cfg->size = toolSizeFromString(sSize);
                } else {
                    // If not specified: do not change
                    cfg->size = TOOL_SIZE_NONE;
                }
            }

            if (type == TOOL_PEN || type == TOOL_HIGHLIGHTER || type == TOOL_TEXT) {
                if (int iColor; e.getInt("color", iColor)) {
                    cfg->color = Color(iColor);
                }
            }

            if (type == TOOL_ERASER) {
                string sEraserMode;
                if (e.getString("eraserMode", sEraserMode)) {
                    cfg->eraserMode = eraserTypeFromString(sEraserMode);
                } else {
                    // If not specified: do not change
                    cfg->eraserMode = ERASER_TYPE_NONE;
                }

                string sSize;
                if (e.getString("size", sSize)) {
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
        } else {
            continue;
        }
    }
}

auto Settings::load() -> bool {
    xmlKeepBlanksDefault(0);

    if (!fs::exists(filepath)) {
        g_warning("configfile does not exist %s\n", filepath.string().c_str());
        return false;
    }

    xmlDocPtr doc = xmlParseFile(filepath.u8string().c_str());

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

    if (xmlStrcmp(cur->name, reinterpret_cast<const xmlChar*>("settings"))) {
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

    /*
     * load Color Palette
     *  - if path does not exist create default palette file
     *  - if error during parsing load default, but do not overwrite
     *    existing palette file (would be annoying for users)
     */
    auto paletteFile = Util::getConfigFile(PALETTE_FILE);
    if (!fs::exists(paletteFile)) {
        Palette::create_default(paletteFile);
    }
    this->palette = std::make_unique<Palette>(std::move(paletteFile));
    try {
        this->palette->load();
    } catch (const std::exception& e) {
        this->palette->parseErrorDialog(e);
        this->palette->load_default();
    }

    return true;
}

auto Settings::savePropertyDouble(const gchar* key, double value, xmlNodePtr parent) -> xmlNodePtr {
    char text[G_ASCII_DTOSTR_BUF_SIZE];
    //  g_ascii_ version uses C locale always.
    g_ascii_formatd(text, G_ASCII_DTOSTR_BUF_SIZE, Util::PRECISION_FORMAT_STRING, value);
    xmlNodePtr xmlNode = saveProperty(key, text, parent);
    return xmlNode;
}

auto Settings::saveProperty(const gchar* key, int value, xmlNodePtr parent) -> xmlNodePtr {
    char* text = g_strdup_printf("%i", value);
    xmlNodePtr xmlNode = saveProperty(key, text, parent);
    g_free(text);
    return xmlNode;
}

auto Settings::savePropertyUnsigned(const gchar* key, unsigned int value, xmlNodePtr parent) -> xmlNodePtr {
    char* text = g_strdup_printf("%u", value);
    xmlNodePtr xmlNode = saveProperty(key, text, parent);
    g_free(text);
    return xmlNode;
}

auto Settings::saveProperty(const gchar* key, const gchar* value, xmlNodePtr parent) -> xmlNodePtr {
    xmlNodePtr xmlNode = xmlNewChild(parent, nullptr, reinterpret_cast<const xmlChar*>("property"), nullptr);

    xmlSetProp(xmlNode, reinterpret_cast<const xmlChar*>("name"), reinterpret_cast<const xmlChar*>(key));

    xmlSetProp(xmlNode, reinterpret_cast<const xmlChar*>("value"), reinterpret_cast<const xmlChar*>(value));

    return xmlNode;
}

void Settings::saveDeviceClasses() {
    SElement& s = getCustomElement("deviceClasses");

    for (auto& device: inputDeviceClasses) {
        const std::string& name = device.first;
        InputDeviceTypeOption& deviceClass = device.second.first;
        GdkInputSource& source = device.second.second;
        SElement& e = s.child(name);
        e.setInt("deviceClass", static_cast<int>(deviceClass));
        e.setInt("deviceSource", source);
    }
}

void Settings::saveButtonConfig() {
    SElement& s = getCustomElement("buttonConfig");
    s.clear();

    for (int i = 0; i < BUTTON_COUNT; i++) {
        SElement& e = s.child(buttonToString(static_cast<Button>(i)));
        const auto& cfg = buttonConfig[i];

        ToolType type = cfg->action;
        e.setString("tool", toolTypeToString(type));

        if (type == TOOL_PEN || type == TOOL_HIGHLIGHTER) {
            e.setString("drawingType", drawingTypeToString(cfg->drawingType));
            e.setString("size", toolSizeToString(cfg->size));
        }  // end if pen or highlighter

        if (type == TOOL_PEN || type == TOOL_HIGHLIGHTER || type == TOOL_TEXT) {
            e.setIntHex("color", int32_t(uint32_t(cfg->color)));
        }

        if (type == TOOL_ERASER) {
            e.setString("eraserMode", eraserTypeToString(cfg->eraserMode));
            e.setString("size", toolSizeToString(cfg->size));
        }

        // Touch device
        if (i == BUTTON_TOUCH) {
            e.setString("device", cfg->device);
            e.setBool("disableDrawing", cfg->disableDrawing);
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

    doc = xmlNewDoc(reinterpret_cast<const xmlChar*>("1.0"));
    if (doc == nullptr) {
        return;
    }

    saveButtonConfig();
    saveDeviceClasses();

    /* Create metadata root */
    root = xmlNewDocNode(doc, nullptr, reinterpret_cast<const xmlChar*>("settings"), nullptr);
    xmlDocSetRootElement(doc, root);
    xmlNodePtr com = xmlNewComment(
            reinterpret_cast<const xmlChar*>("The Xournal++ settings file. Do not edit this file! "
                                             "Most settings are available in the Settings dialog, "
                                             "the others are commented in this file, but handle with care!"));
    xmlAddPrevSibling(root, com);

    SAVE_BOOL_PROP(pressureSensitivity);
    SAVE_DOUBLE_PROP(minimumPressure);
    SAVE_DOUBLE_PROP(pressureMultiplier);

    SAVE_BOOL_PROP(zoomGesturesEnabled);

    SAVE_STRING_PROP(selectedToolbar);

    auto lastSavePath = this->lastSavePath.u8string();
    auto lastOpenPath = this->lastOpenPath.u8string();
    auto lastImagePath = this->lastImagePath.u8string();
    SAVE_STRING_PROP(lastSavePath);
    SAVE_STRING_PROP(lastOpenPath);
    SAVE_STRING_PROP(lastImagePath);

    SAVE_DOUBLE_PROP(edgePanSpeed);
    SAVE_DOUBLE_PROP(edgePanMaxMult);
    SAVE_DOUBLE_PROP(zoomStep);
    SAVE_DOUBLE_PROP(zoomStepScroll);
    SAVE_INT_PROP(displayDpi);
    SAVE_INT_PROP(mainWndWidth);
    SAVE_INT_PROP(mainWndHeight);
    SAVE_BOOL_PROP(maximized);

    SAVE_BOOL_PROP(showToolbar);

    SAVE_BOOL_PROP(showSidebar);
    SAVE_INT_PROP(sidebarWidth);
    xmlNode = saveProperty("sidebarNumberingStyle", static_cast<int>(sidebarNumberingStyle), root);

    SAVE_BOOL_PROP(sidebarOnRight);
    SAVE_BOOL_PROP(scrollbarOnLeft);
    SAVE_BOOL_PROP(menubarVisible);
    SAVE_BOOL_PROP(filepathShownInTitlebar);
    SAVE_INT_PROP(numColumns);
    SAVE_INT_PROP(numRows);
    SAVE_BOOL_PROP(viewFixedRows);
    SAVE_BOOL_PROP(showPairedPages);
    SAVE_BOOL_PROP(layoutVertical);
    SAVE_BOOL_PROP(layoutRightToLeft);
    SAVE_BOOL_PROP(layoutBottomToTop);
    SAVE_INT_PROP(numPairsOffset);
    xmlNode = saveProperty("emptyLastPageAppend", emptyLastPageAppendToString(this->emptyLastPageAppend), root);
    ATTACH_COMMENT("The icon theme, allowed values are \"disabled\", \"onDrawOfLastPage\", and \"onScrollOfLastPage\"");
    SAVE_BOOL_PROP(presentationMode);

    auto defaultViewModeAttributes = viewModeToSettingsString(viewModes.at(PresetViewModeIds::VIEW_MODE_DEFAULT));
    auto fullscreenViewModeAttributes = viewModeToSettingsString(viewModes.at(PresetViewModeIds::VIEW_MODE_FULLSCREEN));
    auto presentationViewModeAttributes =
            viewModeToSettingsString(viewModes.at(PresetViewModeIds::VIEW_MODE_PRESENTATION));
    SAVE_STRING_PROP(defaultViewModeAttributes);
    ATTACH_COMMENT("Which GUI elements are shown in default view mode, separated by a colon (,)");
    SAVE_STRING_PROP(fullscreenViewModeAttributes);
    ATTACH_COMMENT("Which GUI elements are shown in fullscreen view mode, separated by a colon (,)");
    SAVE_STRING_PROP(presentationViewModeAttributes);
    ATTACH_COMMENT("Which GUI elements are shown in presentation view mode, separated by a colon (,)");

    xmlNode = saveProperty("stylusCursorType", stylusCursorTypeToString(this->stylusCursorType), root);
    ATTACH_COMMENT("The cursor icon used with a stylus, allowed values are \"none\", \"dot\", \"big\", \"arrow\"");

    xmlNode = saveProperty("eraserVisibility", eraserVisibilityToString(this->eraserVisibility), root);
    ATTACH_COMMENT("The eraser cursor visibility used with a stylus, allowed values are \"never\", \"always\", "
                   "\"hover\", \"touch\"");

    xmlNode = saveProperty("iconTheme", iconThemeToString(this->iconTheme), root);
    ATTACH_COMMENT("The icon theme, allowed values are \"iconsColor\", \"iconsLucide\"");

    SAVE_BOOL_PROP(highlightPosition);
    xmlNode = savePropertyUnsigned("cursorHighlightColor", uint32_t(cursorHighlightColor), root);
    xmlNode = savePropertyUnsigned("cursorHighlightBorderColor", uint32_t(cursorHighlightBorderColor), root);
    SAVE_DOUBLE_PROP(cursorHighlightRadius);
    SAVE_DOUBLE_PROP(cursorHighlightBorderWidth);
    SAVE_BOOL_PROP(darkTheme);
    SAVE_BOOL_PROP(useStockIcons);

    SAVE_BOOL_PROP(disableScrollbarFadeout);

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

    SAVE_BOOL_PROP(autoloadMostRecent);
    SAVE_BOOL_PROP(autoloadPdfXoj);
    SAVE_STRING_PROP(defaultSaveName);
    SAVE_STRING_PROP(defaultPdfExportName);

    SAVE_BOOL_PROP(autosaveEnabled);
    SAVE_INT_PROP(autosaveTimeout);

    SAVE_BOOL_PROP(addHorizontalSpace);
    SAVE_INT_PROP(addHorizontalSpaceAmount);
    SAVE_BOOL_PROP(addVerticalSpace);
    SAVE_INT_PROP(addVerticalSpaceAmount);

    SAVE_BOOL_PROP(drawDirModsEnabled);
    SAVE_INT_PROP(drawDirModsRadius);


    SAVE_BOOL_PROP(snapRotation);
    SAVE_DOUBLE_PROP(snapRotationTolerance);
    SAVE_BOOL_PROP(snapGrid);
    SAVE_DOUBLE_PROP(snapGridTolerance);
    SAVE_DOUBLE_PROP(snapGridSize);

    SAVE_DOUBLE_PROP(strokeRecognizerMinSize);

    SAVE_BOOL_PROP(touchDrawing);
    SAVE_BOOL_PROP(gtkTouchInertialScrolling);
    SAVE_BOOL_PROP(pressureGuessing);

    xmlNode = savePropertyUnsigned("selectionBorderColor", uint32_t(selectionBorderColor), root);
    xmlNode = savePropertyUnsigned("backgroundColor", uint32_t(backgroundColor), root);
    xmlNode = savePropertyUnsigned("selectionMarkerColor", uint32_t(selectionMarkerColor), root);

    SAVE_DOUBLE_PROP(touchZoomStartThreshold);
    SAVE_DOUBLE_PROP(pageRerenderThreshold);

    SAVE_INT_PROP(pdfPageCacheSize);
    ATTACH_COMMENT("The count of rendered PDF pages which will be cached.");
    SAVE_UINT_PROP(preloadPagesBefore);
    SAVE_UINT_PROP(preloadPagesAfter);
    SAVE_BOOL_PROP(eagerPageCleanup);

    SAVE_STRING_PROP(pageTemplate);
    ATTACH_COMMENT("Config for new pages");

    SAVE_STRING_PROP(sizeUnit);
    {
        auto audioFolder = this->audioFolder.u8string();
        SAVE_STRING_PROP(audioFolder);
    }
    SAVE_INT_PROP(audioInputDevice);
    SAVE_INT_PROP(audioOutputDevice);
    SAVE_DOUBLE_PROP(audioSampleRate);
    SAVE_DOUBLE_PROP(audioGain);
    SAVE_INT_PROP(defaultSeekTime);

    SAVE_STRING_PROP(pluginEnabled);
    SAVE_STRING_PROP(pluginDisabled);

    SAVE_INT_PROP(strokeFilterIgnoreTime);
    SAVE_DOUBLE_PROP(strokeFilterIgnoreLength);
    SAVE_INT_PROP(strokeFilterSuccessiveTime);
    SAVE_BOOL_PROP(strokeFilterEnabled);
    SAVE_BOOL_PROP(doActionOnStrokeFiltered);
    SAVE_BOOL_PROP(trySelectOnStrokeFiltered);

    SAVE_BOOL_PROP(snapRecognizedShapesEnabled);
    SAVE_BOOL_PROP(restoreLineWidthEnabled);

    SAVE_INT_PROP(numIgnoredStylusEvents);

    SAVE_BOOL_PROP(inputSystemTPCButton);
    SAVE_BOOL_PROP(inputSystemDrawOutsideWindow);

    SAVE_STRING_PROP(preferredLocale);

    /**
     * Stabilizer related settings
     */
    saveProperty("stabilizerAveragingMethod", static_cast<int>(stabilizerAveragingMethod), root);
    saveProperty("stabilizerPreprocessor", static_cast<int>(stabilizerPreprocessor), root);
    SAVE_UINT_PROP(stabilizerBuffersize);
    SAVE_DOUBLE_PROP(stabilizerSigma);
    SAVE_DOUBLE_PROP(stabilizerDeadzoneRadius);
    SAVE_DOUBLE_PROP(stabilizerDrag);
    SAVE_DOUBLE_PROP(stabilizerMass);
    SAVE_BOOL_PROP(stabilizerCuspDetection);
    SAVE_BOOL_PROP(stabilizerFinalizeStroke);
    /**/

    SAVE_BOOL_PROP(latexSettings.autoCheckDependencies);
    SAVE_STRING_PROP(latexSettings.defaultText);
    // Inline SAVE_STRING_PROP(latexSettings.globalTemplatePath) since it
    // breaks on Windows due to the native character representation being
    // wchar_t instead of char
    fs::path& p = latexSettings.globalTemplatePath;
    xmlNode = saveProperty("latexSettings.globalTemplatePath", p.empty() ? "" : p.u8string().c_str(), root);
    SAVE_STRING_PROP(latexSettings.genCmd);
    SAVE_STRING_PROP(latexSettings.sourceViewThemeId);
    SAVE_FONT_PROP(latexSettings.editorFont);
    SAVE_BOOL_PROP(latexSettings.useCustomEditorFont);
    SAVE_BOOL_PROP(latexSettings.editorWordWrap);
    SAVE_BOOL_PROP(latexSettings.sourceViewAutoIndent);
    SAVE_BOOL_PROP(latexSettings.sourceViewSyntaxHighlight);
    SAVE_BOOL_PROP(latexSettings.sourceViewShowLineNumbers);

    xmlNodePtr xmlFont = nullptr;
    xmlFont = xmlNewChild(root, nullptr, reinterpret_cast<const xmlChar*>("property"), nullptr);
    xmlSetProp(xmlFont, reinterpret_cast<const xmlChar*>("name"), reinterpret_cast<const xmlChar*>("font"));
    xmlSetProp(xmlFont, reinterpret_cast<const xmlChar*>("font"),
               reinterpret_cast<const xmlChar*>(this->font.getName().c_str()));

    char sSize[G_ASCII_DTOSTR_BUF_SIZE];

    g_ascii_formatd(sSize, G_ASCII_DTOSTR_BUF_SIZE, Util::PRECISION_FORMAT_STRING,
                    this->font.getSize());  // no locale
    xmlSetProp(xmlFont, reinterpret_cast<const xmlChar*>("size"), reinterpret_cast<const xmlChar*>(sSize));


    for (std::map<string, SElement>::value_type p: data) {
        saveData(root, p.first, p.second);
    }

    xmlSaveFormatFileEnc(filepath.u8string().c_str(), doc, "UTF-8", 1);
    xmlFreeDoc(doc);
}

void Settings::saveData(xmlNodePtr root, const string& name, SElement& elem) {
    xmlNodePtr xmlNode = xmlNewChild(root, nullptr, reinterpret_cast<const xmlChar*>("data"), nullptr);

    xmlSetProp(xmlNode, reinterpret_cast<const xmlChar*>("name"), reinterpret_cast<const xmlChar*>(name.c_str()));

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
        at = xmlNewChild(xmlNode, nullptr, reinterpret_cast<const xmlChar*>("attribute"), nullptr);

        xmlSetProp(at, reinterpret_cast<const xmlChar*>("name"), reinterpret_cast<const xmlChar*>(aname.c_str()));
        xmlSetProp(at, reinterpret_cast<const xmlChar*>("type"), reinterpret_cast<const xmlChar*>(type.c_str()));
        xmlSetProp(at, reinterpret_cast<const xmlChar*>("value"), reinterpret_cast<const xmlChar*>(value.c_str()));

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

auto Settings::getAddVerticalSpaceAmount() const -> int { return this->addVerticalSpaceAmount; }

void Settings::setAddVerticalSpaceAmount(int pixels) {
    if (this->addVerticalSpaceAmount == pixels) {
        return;
    }

    this->addVerticalSpaceAmount = pixels;
    save();
}


auto Settings::getAddHorizontalSpace() const -> bool { return this->addHorizontalSpace; }

void Settings::setAddHorizontalSpace(bool space) { this->addHorizontalSpace = space; }

auto Settings::getAddHorizontalSpaceAmount() const -> int { return this->addHorizontalSpaceAmount; }

void Settings::setAddHorizontalSpaceAmount(int pixels) {
    if (this->addHorizontalSpaceAmount == pixels) {
        return;
    }

    this->addHorizontalSpaceAmount = pixels;
    save();
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

auto Settings::getDefaultSaveName() const -> string const& { return this->defaultSaveName; }

void Settings::setDefaultSaveName(const string& name) {
    if (this->defaultSaveName == name) {
        return;
    }

    this->defaultSaveName = name;

    save();
}

auto Settings::getDefaultPdfExportName() const -> string const& { return this->defaultPdfExportName; }

void Settings::setDefaultPdfExportName(const string& name) {
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

auto Settings::getAudioFolder() const -> fs::path const& { return this->audioFolder; }

void Settings::setAudioFolder(fs::path audioFolder) {
    if (this->audioFolder == audioFolder) {
        return;
    }

    this->audioFolder = std::move(audioFolder);

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

void Settings::setPressureSensitivity(gboolean presureSensitivity) {
    if (this->pressureSensitivity == presureSensitivity) {
        return;
    }
    this->pressureSensitivity = presureSensitivity;

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

void Settings::setDarkTheme(bool dark) {
    if (this->darkTheme == dark) {
        return;
    }
    this->darkTheme = dark;
    save();
}

auto Settings::isDarkTheme() const -> bool { return this->darkTheme; }

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

void Settings::setMainWndMaximized(bool max) { this->maximized = max; }

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

void Settings::setViewMode(ViewModeId mode, ViewMode viewMode) { viewModes.at(mode) = viewMode; }

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

void SElement::setIntHex(const string& name, const int value) {
    SAttribute& attrib = this->element->attributes[name];
    attrib.iValue = value;
    attrib.type = ATTRIBUTE_TYPE_INT_HEX;
}

void SElement::setInt(const string& name, const int value) {
    SAttribute& attrib = this->element->attributes[name];
    attrib.iValue = value;
    attrib.type = ATTRIBUTE_TYPE_INT;
}

void SElement::setBool(const string& name, const bool value) {
    SAttribute& attrib = this->element->attributes[name];
    attrib.iValue = value;
    attrib.type = ATTRIBUTE_TYPE_BOOLEAN;
}

void SElement::setString(const string& name, const string& value) {
    SAttribute& attrib = this->element->attributes[name];
    attrib.sValue = value;
    attrib.type = ATTRIBUTE_TYPE_STRING;
}

void SElement::setDouble(const string& name, const double value) {
    SAttribute& attrib = this->element->attributes[name];
    attrib.dValue = value;
    attrib.type = ATTRIBUTE_TYPE_DOUBLE;
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

    value = attrib.iValue;

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

/**
 * @brief Get Color Palette used for Tools
 *
 * @return Palette&
 */
auto Settings::getColorPalette() -> const Palette& { return *(this->palette); }
