/*
 * Xournal++
 *
 * File describing the Settings
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include <array>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <libxml/tree.h>

#include "model/Font.h"
#include "util/Color.h"

#include "ButtonConfig.h"
#include "LatexSettings.h"
#include "Settings.h"
#include "SettingsEnums.h"
#include "ViewModes.h"
#include "filesystem.h"

class SElement;
class ButtonConfig;

/*
 * In order to add settings add an element to the SettingsElement enum
 * after that add the specialization of the Setting struct further below
 *
 */

enum class SettingsElement {
    FONT,
    PRESSURE_SENSITIVITY,
    MINIMUM_PRESSURE,
    PRESSURE_MULTIPLIER,
    ENABLE_ZOOM_GESTURES,
    SELECTED_TOOLBAR,
    LAST_SAVE_PATH,
    LAST_OPEN_PATH,
    LAST_IMAGE_PATH,
    EDGE_PAN_SPEED,
    EDGE_PAN_MAX_MULT,
    ZOOM_STEP,
    ZOOM_STEP_SCROLL,
    DISPLAY_DPI,
    MAIN_WINDOW_WIDTH,
    MAIN_WINDOW_HEIGHT,
    MAXIMIZED,
    SHOW_TOOLBAR,
    SHOW_FILEPATH_IN_TITLEBAR,
    SHOW_PAGE_NUMBER_IN_TITLEBAR,
    SHOW_SIDEBAR,
    SIDEBAR_NUMBERING_STYLE,
    SIDEBAR_WIDTH,
    SIDEBAR_ON_RIGHT,
    SCROLLBAR_ON_LEFT,
    MENUBAR_VISIBLE,
    NUM_COLUMNS,
    NUM_ROWS,
    VIEW_FIXED_ROWS,
    LAYOUT_VERTICAL,
    LAYOUT_RIGHT_TO_LEFT,
    LAYOUT_BOTTOM_TO_TOP,
    SHOW_PAIRED_PAGES,
    NUM_PAIRS_OFFSET,
    AUTOLOAD_MOST_RECENT,
    AUTOLOAD_PDF_XOJ,
    STYLUS_CURSOR_TYPE,
    ERASER_VISIBILITY,
    ICON_THEME,
    HIGHLIGHT_POSITION,
    CURSOR_HIGHLIGHT_COLOR,
    CURSOR_HIGHLIGHT_RADIUS,
    CURSOR_HIGHLIGHT_BORDER_COLOR,
    CURSOR_HIGHLIGHT_BORDER_WIDTH,
    DARK_THEME,
    USE_STOCK_ICONS,
    DEFAULT_SAVE_NAME,
    DEFAULT_PDF_EXPORT_NAME,
    PLUGIN_ENABLED,
    PLUGIN_DISABLED,
    PAGE_TEMPLATE,
    SIZE_UNIT,
    AUDIO_FOLDER,
    AUTOSAVE_ENABLED,
    AUTOSAVE_TIMEOUT,
    ACTIVE_VIEW_MODE,
    DEFAULT_VIEW_MODE_ATTRIBUTES,
    FULLSCREEN_VIEW_MODE_ATTRIBUTES,
    PRESENTATION_VIEW_MODE_ATTRIBUTES,
    TOUCH_ZOOM_START_THRESHOLD,
    PAGE_RERENDER_THRESHOLD,
    PDF_PAGE_CACHE_SIZE,
    PRELOAD_PAGES_BEFORE,
    PRELOAD_PAGES_AFTER,
    EAGER_PAGE_CLEANUP,
    SELECTION_BORDER_COLOR,
    SELECTION_MARKER_COLOR,
    ACTIVE_SELECTION_COLOR,
    BACKGROUND_COLOR,
    ADD_HORIZONTAL_SPACE,
    ADD_HORIZONTAL_SPACE_AMOUNT_RIGHT,
    ADD_HORIZONTAL_SPACE_AMOUNT_LEFT,
    ADD_VERTICAL_SPACE,
    ADD_VERTICAL_SPACE_AMOUNT_ABOVE,
    ADD_VERTICAL_SPACE_AMOUNT_BELOW,
    UNLIMITED_SCROLLING,
    DRAW_DIRECTION_MODS_ENABLE,
    DRAW_DIRECTION_MODS_RADIUS,
    SNAP_ROTATION,
    SNAP_ROTATION_TOLERANCE,
    SNAP_GRID,
    SNAP_GRID_SIZE,
    SNAP_GRID_TOLERANCE,
    STROKE_RECOGNIZER_MIN_SIZE,
    TOUCH_DRAWING,
    GTK_TOUCH_INERTIAL_SCROLLING,
    PRESSURE_GUESSING,
    SCROLLBAR_HIDE_TYPE,
    DISABLE_SCROLLBAR_FADEOUT,
    DISABLE_AUDIO,
    AUDIO_SAMPLE_RATE,
    AUDIO_GAIN,
    DEFAULT_SEEK_TIME,
    AUDIO_INPUT_DEVICE,
    AUDIO_OUTPUT_DEVICE,
    NUM_IGNORED_STYLUS_EVENTS,
    INPUT_SYSTEM_TPC_BUTTON,
    INPUT_SYSTEM_DRAW_OUTSIDE_WINDOW,
    EMPTY_LAST_PAGE_APPEND,
    STROKE_FILTER_IGNORE_TIME,
    STROKE_FILTER_IGNORE_LENGTH,
    STROKE_FILTER_SUCCESSIVE_TIME,
    STROKE_FILTER_ENABLED,
    DO_ACTION_ON_STROKE_FILTERED,
    TRY_SELECT_ON_STROKE_FILTERED,
    LATEX_SETTINGS,
    SNAP_RECOGNIZED_SHAPES,
    RESTORE_LINE_WIDTH,
    PREFERRED_LOCALE,
    STABILIZER_AVERAGING_METHOD,
    STABILIZER_PREPROCESSOR,
    STABILIZER_BUFFERSIZE,
    STABILIZER_SIGMA,
    STABILIZER_DEADZONE_RADIUS,
    STABILIZER_DRAG,
    STABILIZER_MASS,
    STABILIZER_CUSP_DETECTION,
    STABILIZER_FINALIZE_STROKE,
    USE_SPACES_AS_TAB,
    NUMBER_OF_SPACES_FOR_TAB,
    // Nested Settings from here on
    // Saved in data tags in the settings file
    NESTED_TOUCH,
    NESTED_LAST_USED_PAGE_BACKGROUND_COLOR,
    NESTED_DEVICE_CLASSES,
    NESTED_BUTTON_CONFIG,
    NESTED_TOOLS,
    // Don't add more entries below this comment
    ENUM_COUNT
};


// Definition of import function template
template <typename T>
bool importProperty(xmlNodePtr node, T& var);


// Definition of export function template
template <typename T>
xmlNodePtr exportProperty(xmlNodePtr parent, std::string name, T value);

template <>
xmlNodePtr exportProperty(xmlNodePtr node, std::string name, int value);

// Definitions of helper functions
xmlNodePtr exportProp(xmlNodePtr parent, const char* name, const char* value);
bool importButtonConfig(xmlNodePtr node, std::array<std::shared_ptr<ButtonConfig>, BUTTON_COUNT>& var);
bool importDeviceClasses(xmlNodePtr node, std::map<std::string, std::pair<InputDeviceTypeOption, GdkInputSource>>& var);
bool importSidebarNumberingStyle(xmlNodePtr node, SidebarNumberingStyle& var);
bool importStylusCursorType(xmlNodePtr node, StylusCursorType& var);
bool importEraserVisibility(xmlNodePtr node, EraserVisibility& var);
bool importIconTheme(xmlNodePtr node, IconTheme& var);
bool importScrollbarHideType(xmlNodePtr node, ScrollbarHideType& var);
bool importEmptyLastPageAppendType(xmlNodePtr node, EmptyLastPageAppendType& var);
bool importLatexSettings(xmlNodePtr node, LatexSettings& var);
bool importAveragingMethod(xmlNodePtr node, StrokeStabilizer::AveragingMethod& var);
bool importPreprocessor(xmlNodePtr node, StrokeStabilizer::Preprocessor& var);
bool importFont(xmlNodePtr node, XojFont& var);

xmlNodePtr exportButtonConfig(xmlNodePtr node, std::string name,
                              const std::array<std::shared_ptr<ButtonConfig>, BUTTON_COUNT>& value);
xmlNodePtr exportDeviceClasses(xmlNodePtr node, std::string name,
                               const std::map<std::string, std::pair<InputDeviceTypeOption, GdkInputSource>>& value);
xmlNodePtr exportScrollbarHideType(xmlNodePtr node, std::string name, ScrollbarHideType value);
xmlNodePtr exportLatexSettings(xmlNodePtr node, std::string name, const LatexSettings& value);
xmlNodePtr exportFont(xmlNodePtr node, std::string name, const XojFont& value);


// Definitions of getter return types for value types
template <typename T>
struct getter_return {
    using type = T;
};
template <>
struct getter_return<std::string> {
    using type = const std::string&;
};
template <>
struct getter_return<fs::path> {
    using type = const fs::path&;
};
template <>
struct getter_return<XojFont> {
    using type = const XojFont&;
};
template <>
struct getter_return<LatexSettings> {
    using type = const LatexSettings&;
};
template <typename T, size_t N>
struct getter_return<std::array<T, N>> {
    using type = const std::array<T, N>&;
};
template <typename T, typename U>
struct getter_return<std::map<T, U>> {
    using type = const std::map<T, U>&;
};
template <>
struct getter_return<SElement> {
    using type = const SElement&;
};

template <typename T>
using getter_return_t = typename getter_return<T>::type;


// Definition of single Settings
template <SettingsElement>
struct Setting {};

// Definitions of SettingsElements
/* Structs describing each setting
 *
 * Define like this:
 * template<>
 * struct Setting<*The name of your settings element*> {
 *    value_type: The type your settings value should have.
 *
 *    XML_NAME: The name you want for your property in the file.
 *
 *    DEFAULT: The default value of your Setting, if your value_type is not a literal type leave it
 *             unset here, and add the initialization in SettingsDescriptions.cpp at the top.
 *
 *    COMMENT: The comment in the file above your setting, if not needed leave this out of the
 *             definition of the struct.
 *
 *    IMPORT_FN: Function that reads a settings value from the xml tag.
 *               Only required if the setting is not of a type that has a default import function.
 *               Function signature: (xmlNodePtr, value_type&) -> bool
 *                               tag from file^ | ^save value here | ^operation success
 *
 *    EXPORT_FN: Function that writes a setting value into an xml tag and adds it to the document root.
 *               Only required if the setting is not of a type that has a default export function.
 *               Function signature: (xmlNodePtr, std::string, getter_return_t<value_type>) -> xmlNodePtr
 *                                    ^parent   | ^tag name  | ^value to be saved            | ^node that was added
 *
 *    VALIDATE_FN: Function that checks if the value is correct and always returns a valid value,
 *                 called at import and setter.
 *                 Only required if the settings value needs to be checked.
 *                 Function signature: (value_type) -> value_type
 *                                      ^value to be checked | ^corrected value
 *
 * };
 */

template <>
struct Setting<SettingsElement::FONT> {
    using value_type = XojFont;
    static constexpr auto XML_NAME = "font";
    static const value_type DEFAULT;
    static constexpr auto IMPORT_FN = importFont;
    static constexpr auto EXPORT_FN = exportFont;
};

template <>
struct Setting<SettingsElement::PRESSURE_SENSITIVITY> {
    using value_type = bool;
    static constexpr auto XML_NAME = "pressureSensitivity";
    static constexpr value_type DEFAULT = true;
};

template <>
struct Setting<SettingsElement::MINIMUM_PRESSURE> {
    using value_type = double;
    static constexpr auto XML_NAME = "minimumPressure";
    static constexpr value_type DEFAULT = 0.05;
    static constexpr auto VALIDATE_FN = [](value_type val) -> value_type { return std::max<value_type>(val, 0.01); };
};

template <>
struct Setting<SettingsElement::PRESSURE_MULTIPLIER> {
    using value_type = double;
    static constexpr auto XML_NAME = "pressureMultiplier";
    static constexpr value_type DEFAULT = 1.0;
};

template <>
struct Setting<SettingsElement::ENABLE_ZOOM_GESTURES> {
    using value_type = bool;
    static constexpr auto XML_NAME = "zoomGesturesEnabled";
    static constexpr value_type DEFAULT = true;
};

template <>
struct Setting<SettingsElement::SELECTED_TOOLBAR> {
    using value_type = std::string;
    static constexpr auto XML_NAME = "selectedToolbar";
    static constexpr const char* DEFAULT = "Portrait";
};

template <>
struct Setting<SettingsElement::LAST_SAVE_PATH> {
    using value_type = fs::path;
    static constexpr auto XML_NAME = "lastSavePath";
    static constexpr const char* DEFAULT = "";
};

template <>
struct Setting<SettingsElement::LAST_OPEN_PATH> {
    using value_type = fs::path;
    static constexpr auto XML_NAME = "lastOpenPath";
    static constexpr const char* DEFAULT = "";
};

template <>
struct Setting<SettingsElement::LAST_IMAGE_PATH> {
    using value_type = fs::path;
    static constexpr auto XML_NAME = "lastImagePath";
    static constexpr const char* DEFAULT = "";
};

template <>
struct Setting<SettingsElement::EDGE_PAN_SPEED> {
    using value_type = double;
    static constexpr auto XML_NAME = "edgePanSpeed";
    static constexpr value_type DEFAULT = 20.0;
};

template <>
struct Setting<SettingsElement::EDGE_PAN_MAX_MULT> {
    using value_type = double;
    static constexpr auto XML_NAME = "edgePanMaxMult";
    static constexpr value_type DEFAULT = 5.0;
};

template <>
struct Setting<SettingsElement::ZOOM_STEP> {
    using value_type = double;
    static constexpr auto XML_NAME = "zoomStep";
    static constexpr value_type DEFAULT = 10.0;
};

template <>
struct Setting<SettingsElement::ZOOM_STEP_SCROLL> {
    using value_type = double;
    static constexpr auto XML_NAME = "zoomStepScroll";
    static constexpr value_type DEFAULT = 2.0;
};

template <>
struct Setting<SettingsElement::DISPLAY_DPI> {
    using value_type = int;
    static constexpr auto XML_NAME = "displayDpi";
    static constexpr value_type DEFAULT = 72;
};

template <>
struct Setting<SettingsElement::MAIN_WINDOW_WIDTH> {
    using value_type = int;
    static constexpr auto XML_NAME = "mainWndWidth";
    static constexpr value_type DEFAULT = 800;
};

template <>
struct Setting<SettingsElement::MAIN_WINDOW_HEIGHT> {
    using value_type = int;
    static constexpr auto XML_NAME = "mainWndHeight";
    static constexpr value_type DEFAULT = 600;
};

template <>
struct Setting<SettingsElement::MAXIMIZED> {
    using value_type = bool;
    static constexpr auto XML_NAME = "maximized";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::SHOW_TOOLBAR> {
    using value_type = bool;
    static constexpr auto XML_NAME = "showToolbar";
    static constexpr value_type DEFAULT = true;
};

template <>
struct Setting<SettingsElement::SHOW_FILEPATH_IN_TITLEBAR> {
    using value_type = bool;
    static constexpr auto XML_NAME = "filepathShownInTitlebar";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::SHOW_PAGE_NUMBER_IN_TITLEBAR> {
    using value_type = bool;
    static constexpr auto XML_NAME = "pageNumberShownInTitlebar";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::SHOW_SIDEBAR> {
    using value_type = bool;
    static constexpr auto XML_NAME = "showSidebar";
    static constexpr value_type DEFAULT = true;
};

template <>
struct Setting<SettingsElement::SIDEBAR_NUMBERING_STYLE> {
    using value_type = SidebarNumberingStyle;
    static constexpr auto XML_NAME = "sidebarNumberingStyle";
    static constexpr value_type DEFAULT = SidebarNumberingStyle::DEFAULT;
    static constexpr auto IMPORT_FN = importSidebarNumberingStyle;
    static constexpr auto EXPORT_FN = [](xmlNodePtr node, std::string name, SidebarNumberingStyle value) {
        return exportProperty(node, name, static_cast<int>(value));
    };
    static constexpr auto VALIDATE_FN = [](value_type val) -> value_type {
        return std::max<value_type>(SidebarNumberingStyle::MIN, std::min<value_type>(val, SidebarNumberingStyle::MAX));
    };
};

template <>
struct Setting<SettingsElement::SIDEBAR_WIDTH> {
    using value_type = int;
    static constexpr auto XML_NAME = "sidebarWidth";
    static constexpr value_type DEFAULT = 150;
    static constexpr auto VALIDATE_FN = [](value_type val) -> value_type { return std::max<value_type>(val, 50); };
};

template <>
struct Setting<SettingsElement::SIDEBAR_ON_RIGHT> {
    using value_type = bool;
    static constexpr auto XML_NAME = "sidebarOnRight";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::SCROLLBAR_ON_LEFT> {
    using value_type = bool;
    static constexpr auto XML_NAME = "scrollbarOnLeft";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::MENUBAR_VISIBLE> {
    using value_type = bool;
    static constexpr auto XML_NAME = "menubarVisible";
    static constexpr value_type DEFAULT = true;
};

template <>
struct Setting<SettingsElement::NUM_COLUMNS> {
    using value_type = int;
    static constexpr auto XML_NAME = "numColumns";
    static constexpr value_type DEFAULT = 1;
};

template <>
struct Setting<SettingsElement::NUM_ROWS> {
    using value_type = int;
    static constexpr auto XML_NAME = "numRows";
    static constexpr value_type DEFAULT = 1;
};

template <>
struct Setting<SettingsElement::VIEW_FIXED_ROWS> {
    using value_type = bool;
    static constexpr auto XML_NAME = "viewFixedRows";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::LAYOUT_VERTICAL> {
    using value_type = bool;
    static constexpr auto XML_NAME = "layoutVertical";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::LAYOUT_RIGHT_TO_LEFT> {
    using value_type = bool;
    static constexpr auto XML_NAME = "layoutRightToLeft";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::LAYOUT_BOTTOM_TO_TOP> {
    using value_type = bool;
    static constexpr auto XML_NAME = "layoutBottomToTop";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::SHOW_PAIRED_PAGES> {
    using value_type = bool;
    static constexpr auto XML_NAME = "showPairedPages";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::NUM_PAIRS_OFFSET> {
    using value_type = int;
    static constexpr auto XML_NAME = "numPairsOffset";
    static constexpr value_type DEFAULT = 1;
};

template <>
struct Setting<SettingsElement::AUTOLOAD_MOST_RECENT> {
    using value_type = bool;
    static constexpr auto XML_NAME = "autoloadMostRecent";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::AUTOLOAD_PDF_XOJ> {
    using value_type = bool;
    static constexpr auto XML_NAME = "autoloadPdfXoj";
    static constexpr value_type DEFAULT = true;
};

template <>
struct Setting<SettingsElement::STYLUS_CURSOR_TYPE> {
    using value_type = StylusCursorType;
    static constexpr auto XML_NAME = "stylusCursorType";
    static constexpr value_type DEFAULT = StylusCursorType::STYLUS_CURSOR_DOT;
    static constexpr auto COMMENT =
            "The cursor icon used with a stylus, allowed values are \"none\", \"dot\", \"big\", \"arrow\"";
    static constexpr auto IMPORT_FN = importStylusCursorType;
    static constexpr auto EXPORT_FN = [](xmlNodePtr node, std::string name, StylusCursorType value) {
        return exportProp(node, name.c_str(), stylusCursorTypeToString(value));
    };
};

template <>
struct Setting<SettingsElement::ERASER_VISIBILITY> {
    using value_type = EraserVisibility;
    static constexpr auto XML_NAME = "eraserVisibility";
    static constexpr value_type DEFAULT = EraserVisibility::ERASER_VISIBILITY_ALWAYS;
    static constexpr auto COMMENT = "The eraser cursor visibility used with a stylus, allowed values are \"never\", "
                                    "\"always\", \"hover\", \"touch\"";
    static constexpr auto IMPORT_FN = importEraserVisibility;
    static constexpr auto EXPORT_FN = [](xmlNodePtr node, std::string name, EraserVisibility value) {
        return exportProp(node, name.c_str(), eraserVisibilityToString(value));
    };
};

template <>
struct Setting<SettingsElement::ICON_THEME> {
    using value_type = IconTheme;
    static constexpr auto XML_NAME = "iconTheme";
    static constexpr value_type DEFAULT = IconTheme::ICON_THEME_COLOR;
    static constexpr auto COMMENT = "The icon theme, allowed values are \"iconsColor\", \"iconsLucide\"";
    static constexpr auto IMPORT_FN = importIconTheme;
    static constexpr auto EXPORT_FN = [](xmlNodePtr node, std::string name, IconTheme value) {
        return exportProp(node, name.c_str(), iconThemeToString(value));
    };
};

template <>
struct Setting<SettingsElement::HIGHLIGHT_POSITION> {
    using value_type = bool;
    static constexpr auto XML_NAME = "highlightPosition";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::CURSOR_HIGHLIGHT_COLOR> {
    using value_type = Color;
    static constexpr auto XML_NAME = "cursorHighlightColor";
    static constexpr value_type DEFAULT = ColorU8(0x80FFFF00);
};

template <>
struct Setting<SettingsElement::CURSOR_HIGHLIGHT_RADIUS> {
    using value_type = double;
    static constexpr auto XML_NAME = "cursorHighlightRadius";
    static constexpr value_type DEFAULT = 30.0;
};

template <>
struct Setting<SettingsElement::CURSOR_HIGHLIGHT_BORDER_COLOR> {
    using value_type = Color;
    static constexpr auto XML_NAME = "cursorHighlightBorderColor";
    static constexpr value_type DEFAULT = ColorU8(0x800000FF);
};

template <>
struct Setting<SettingsElement::CURSOR_HIGHLIGHT_BORDER_WIDTH> {
    using value_type = double;
    static constexpr auto XML_NAME = "cursorHighlightBorderWidth";
    static constexpr value_type DEFAULT = 0.0;
};

template <>
struct Setting<SettingsElement::DARK_THEME> {
    using value_type = bool;
    static constexpr auto XML_NAME = "darkTheme";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::USE_STOCK_ICONS> {
    using value_type = bool;
    static constexpr auto XML_NAME = "useStockIcons";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::DEFAULT_SAVE_NAME> {
    using value_type = std::string;
    static constexpr auto XML_NAME = "defaultSaveName";
    static const value_type DEFAULT;
};

template <>
struct Setting<SettingsElement::DEFAULT_PDF_EXPORT_NAME> {
    using value_type = std::string;
    static constexpr auto XML_NAME = "defaultPdfExportName";
    static const value_type DEFAULT;
};

template <>
struct Setting<SettingsElement::PLUGIN_ENABLED> {
    using value_type = std::string;
    static constexpr auto XML_NAME = "pluginEnabled";
    static constexpr const char* DEFAULT = "";
};

template <>
struct Setting<SettingsElement::PLUGIN_DISABLED> {
    using value_type = std::string;
    static constexpr auto XML_NAME = "pluginDisabled";
    static constexpr const char* DEFAULT = "";
};

template <>
struct Setting<SettingsElement::PAGE_TEMPLATE> {
    using value_type = std::string;
    static constexpr auto XML_NAME = "pageTemplate";
    static constexpr const char* DEFAULT = "xoj/"
                                           "template\ncopyLastPageSettings=true\nsize=595.275591x841."
                                           "889764\nbackgroundType=lined\nbackgroundColor=#ffffff\n";
    static constexpr auto COMMENT = "Config for new pages";
};

template <>
struct Setting<SettingsElement::SIZE_UNIT> {
    using value_type = std::string;
    static constexpr auto XML_NAME = "sizeUnit";
    static constexpr const char* DEFAULT = "";
};

template <>
struct Setting<SettingsElement::AUDIO_FOLDER> {
    using value_type = fs::path;
    static constexpr auto XML_NAME = "audioFolder";
    static constexpr const char* DEFAULT = "";
};

template <>
struct Setting<SettingsElement::AUTOSAVE_ENABLED> {
    using value_type = bool;
    static constexpr auto XML_NAME = "autosaveEnabled";
    static constexpr value_type DEFAULT = true;
};

template <>
struct Setting<SettingsElement::AUTOSAVE_TIMEOUT> {
    using value_type = int;
    static constexpr auto XML_NAME = "autosaveTimeout";
    static constexpr value_type DEFAULT = 3;
};

template <>
struct Setting<SettingsElement::ACTIVE_VIEW_MODE> {
    using value_type = ViewModeId;
    static constexpr auto XML_NAME = "";  // This setting is not saved to the config file
    static constexpr value_type DEFAULT = PresetViewModeIds::VIEW_MODE_DEFAULT;
    static constexpr auto IMPORT_FN = [](xmlNodePtr node, value_type& val) -> bool {
        return false;  // This setting is not stored in the file, returning false makes sure the default is loaded
    };
    static constexpr auto EXPORT_FN = [](xmlNodePtr node, std::string name, value_type& val) -> xmlNodePtr {
        return nullptr;
    };
    static constexpr auto VALIDATE_FN = [](value_type val) -> value_type {
        if (val == PresetViewModeIds::VIEW_MODE_DEFAULT || val == PresetViewModeIds::VIEW_MODE_FULLSCREEN ||
            val == PresetViewModeIds::VIEW_MODE_PRESENTATION) {
            return val;
        }
        return PresetViewModeIds::VIEW_MODE_DEFAULT;
    };
};

template <>
struct Setting<SettingsElement::DEFAULT_VIEW_MODE_ATTRIBUTES> {
    using value_type = ViewMode;
    static constexpr auto XML_NAME = "defaultViewModeAttributes";
    static constexpr value_type DEFAULT = VIEW_MODE_STRUCT_DEFAULT;
    static constexpr auto COMMENT = "Which GUI elements are shown in default view mode, separated by a colon (,)";
};

template <>
struct Setting<SettingsElement::FULLSCREEN_VIEW_MODE_ATTRIBUTES> {
    using value_type = ViewMode;
    static constexpr auto XML_NAME = "fullscreenViewModeAttributes";
    static constexpr value_type DEFAULT = VIEW_MODE_STRUCT_FULLSCREEN;
    static constexpr auto COMMENT = "Which GUI elements are shown in fullscreen view mode, separated by a colon (,)";
};

template <>
struct Setting<SettingsElement::PRESENTATION_VIEW_MODE_ATTRIBUTES> {
    using value_type = ViewMode;
    static constexpr auto XML_NAME = "presentationViewModeAttributes";
    static constexpr value_type DEFAULT = VIEW_MODE_STRUCT_PRESENTATION;
    static constexpr auto COMMENT = "Which GUI elements are shown in presentation view mode, separated by a colon (,)";
};

template <>
struct Setting<SettingsElement::TOUCH_ZOOM_START_THRESHOLD> {
    using value_type = double;
    static constexpr auto XML_NAME = "touchZoomStartThreshold";
    static constexpr value_type DEFAULT = 0.0;
};

template <>
struct Setting<SettingsElement::PAGE_RERENDER_THRESHOLD> {
    using value_type = double;
    static constexpr auto XML_NAME = "pageRerenderThreshold";
    static constexpr value_type DEFAULT = 5.0;
};

template <>
struct Setting<SettingsElement::PDF_PAGE_CACHE_SIZE> {
    using value_type = int;
    static constexpr auto XML_NAME = "pdfPageCacheSize";
    static constexpr value_type DEFAULT = 10;
    static constexpr auto COMMENT = "The count of rendered PDF pages which will be cached.";
};

template <>
struct Setting<SettingsElement::PRELOAD_PAGES_BEFORE> {
    using value_type = uint;
    static constexpr auto XML_NAME = "preloadPagesBefore";
    static constexpr value_type DEFAULT = 3;
};

template <>
struct Setting<SettingsElement::PRELOAD_PAGES_AFTER> {
    using value_type = uint;
    static constexpr auto XML_NAME = "preloadPagesAfter";
    static constexpr value_type DEFAULT = 5;
};

template <>
struct Setting<SettingsElement::EAGER_PAGE_CLEANUP> {
    using value_type = bool;
    static constexpr auto XML_NAME = "eagerPageCleanup";
    static constexpr value_type DEFAULT = true;
};

template <>
struct Setting<SettingsElement::SELECTION_BORDER_COLOR> {
    using value_type = Color;
    static constexpr auto XML_NAME = "selectionBorderColor";
    static constexpr value_type DEFAULT = Colors::red;
};

template <>
struct Setting<SettingsElement::SELECTION_MARKER_COLOR> {
    using value_type = Color;
    static constexpr auto XML_NAME = "selectionMarkerColor";
    static constexpr value_type DEFAULT = Colors::xopp_cornflowerblue;
};

template <>
struct Setting<SettingsElement::ACTIVE_SELECTION_COLOR> {
    using value_type = Color;
    static constexpr auto XML_NAME = "activeSelectionColor";
    static constexpr value_type DEFAULT = Colors::lawngreen;
};

template <>
struct Setting<SettingsElement::BACKGROUND_COLOR> {
    using value_type = Color;
    static constexpr auto XML_NAME = "backgroundColor";
    static constexpr value_type DEFAULT = Colors::xopp_gainsboro02;
};

template <>
struct Setting<SettingsElement::ADD_HORIZONTAL_SPACE> {
    using value_type = bool;
    static constexpr auto XML_NAME = "addHorizontalSpace";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::ADD_HORIZONTAL_SPACE_AMOUNT_RIGHT> {
    using value_type = int;
    static constexpr auto XML_NAME = "addHorizontalSpaceAmountRight";
    static constexpr value_type DEFAULT = 150;
};

template <>
struct Setting<SettingsElement::ADD_HORIZONTAL_SPACE_AMOUNT_LEFT> {
    using value_type = int;
    static constexpr auto XML_NAME = "addHorizontalSpaceAmountLeft";
    static constexpr value_type DEFAULT = 150;
};

template <>
struct Setting<SettingsElement::ADD_VERTICAL_SPACE> {
    using value_type = bool;
    static constexpr auto XML_NAME = "addVerticalSpace";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::ADD_VERTICAL_SPACE_AMOUNT_ABOVE> {
    using value_type = int;
    static constexpr auto XML_NAME = "addVerticalSpaceAmountAbove";
    static constexpr value_type DEFAULT = 150;
};

template <>
struct Setting<SettingsElement::ADD_VERTICAL_SPACE_AMOUNT_BELOW> {
    using value_type = int;
    static constexpr auto XML_NAME = "addVerticalSpaceAmountBelow";
    static constexpr value_type DEFAULT = 150;
};

template <>
struct Setting<SettingsElement::UNLIMITED_SCROLLING> {
    using value_type = bool;
    static constexpr auto XML_NAME = "unlimitedScrolling";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::DRAW_DIRECTION_MODS_ENABLE> {
    using value_type = bool;
    static constexpr auto XML_NAME = "drawDirModsEnabled";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::DRAW_DIRECTION_MODS_RADIUS> {
    using value_type = int;
    static constexpr auto XML_NAME = "drawDirModsRadius";
    static constexpr value_type DEFAULT = 50;
};

template <>
struct Setting<SettingsElement::SNAP_ROTATION> {
    using value_type = bool;
    static constexpr auto XML_NAME = "snapRotation";
    static constexpr value_type DEFAULT = true;
};

template <>
struct Setting<SettingsElement::SNAP_ROTATION_TOLERANCE> {
    using value_type = double;
    static constexpr auto XML_NAME = "snapRotationTolerance";
    static constexpr value_type DEFAULT = 0.3;
};

template <>
struct Setting<SettingsElement::SNAP_GRID> {
    using value_type = bool;
    static constexpr auto XML_NAME = "snapGrid";
    static constexpr value_type DEFAULT = true;
};

template <>
struct Setting<SettingsElement::SNAP_GRID_SIZE> {
    using value_type = double;
    static constexpr auto XML_NAME = "snapGridSize";
    static constexpr value_type DEFAULT = 14.17;
};

template <>
struct Setting<SettingsElement::SNAP_GRID_TOLERANCE> {
    using value_type = double;
    static constexpr auto XML_NAME = "snapGridTolerance";
    static constexpr value_type DEFAULT = 0.50;
};

template <>
struct Setting<SettingsElement::STROKE_RECOGNIZER_MIN_SIZE> {
    using value_type = double;
    static constexpr auto XML_NAME = "strokeRecognizerMinSize";
    static constexpr value_type DEFAULT = 40.0;
};

template <>
struct Setting<SettingsElement::TOUCH_DRAWING> {
    using value_type = bool;
    static constexpr auto XML_NAME = "touchDrawing";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::GTK_TOUCH_INERTIAL_SCROLLING> {
    using value_type = bool;
    static constexpr auto XML_NAME = "gtkTouchInertialScrolling";
    static constexpr value_type DEFAULT = true;
};

template <>
struct Setting<SettingsElement::PRESSURE_GUESSING> {
    using value_type = bool;
    static constexpr auto XML_NAME = "pressureGuessing";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::SCROLLBAR_HIDE_TYPE> {
    using value_type = ScrollbarHideType;
    static constexpr auto XML_NAME = "scrollbarHideType";
    static constexpr value_type DEFAULT = ScrollbarHideType::SCROLLBAR_HIDE_NONE;
    static constexpr auto COMMENT =
            "Hides scroolbars in the main window, allowed values: \"none\", \"horizontal\", \"vertical\", \"both\"";
    static constexpr auto IMPORT_FN = importScrollbarHideType;
    static constexpr auto EXPORT_FN = exportScrollbarHideType;
};

template <>
struct Setting<SettingsElement::DISABLE_SCROLLBAR_FADEOUT> {
    using value_type = bool;
    static constexpr auto XML_NAME = "disableScrollbarFadeout";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::DISABLE_AUDIO> {
    using value_type = bool;
    static constexpr auto XML_NAME = "disableAudio";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::AUDIO_SAMPLE_RATE> {
    using value_type = double;
    static constexpr auto XML_NAME = "audioSampleRate";
    static constexpr value_type DEFAULT = 44100.0;
};

template <>
struct Setting<SettingsElement::AUDIO_GAIN> {
    using value_type = double;
    static constexpr auto XML_NAME = "audioGain";
    static constexpr value_type DEFAULT = 1.0;
};

template <>
struct Setting<SettingsElement::DEFAULT_SEEK_TIME> {
    using value_type = uint;
    static constexpr auto XML_NAME = "defaultSeekTime";
    static constexpr value_type DEFAULT = 5;
};

template <>
struct Setting<SettingsElement::AUDIO_INPUT_DEVICE> {
    using value_type = PaDeviceIndex;
    static constexpr auto XML_NAME = "audioInputDevice";
    static constexpr value_type DEFAULT = -1;  // Value formerly in AUDIO_INPUT_SYSTEM_DEFAULT
};

template <>
struct Setting<SettingsElement::AUDIO_OUTPUT_DEVICE> {
    using value_type = PaDeviceIndex;
    static constexpr auto XML_NAME = "audioOutputDevice";
    static constexpr value_type DEFAULT = -1;  // Value formerly in AUDIO_OUTPUT_SYSTEM_DEFAULT
};

template <>
struct Setting<SettingsElement::NUM_IGNORED_STYLUS_EVENTS> {
    using value_type = int;
    static constexpr auto XML_NAME = "numIgnoredStylusEvents";
    static constexpr value_type DEFAULT = 0;
    static constexpr auto VALIDATE_FN = [](value_type val) -> value_type { return std::max<value_type>(val, 50); };
};

template <>
struct Setting<SettingsElement::INPUT_SYSTEM_TPC_BUTTON> {
    using value_type = bool;
    static constexpr auto XML_NAME = "inputSystemTPCButton";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::INPUT_SYSTEM_DRAW_OUTSIDE_WINDOW> {
    using value_type = bool;
    static constexpr auto XML_NAME = "inputSystemDrawOutsideWindow";
    static constexpr value_type DEFAULT = true;
};

template <>
struct Setting<SettingsElement::EMPTY_LAST_PAGE_APPEND> {
    using value_type = EmptyLastPageAppendType;
    static constexpr auto XML_NAME = "emptyLastPageAppend";
    static constexpr value_type DEFAULT = EmptyLastPageAppendType::Disabled;
    static constexpr auto COMMENT = "empty Last Page Append Type, allowed values are \"disabled\", "
                                    "\"onDrawOfLastPage\", and \"onScrollOfLastPage\"";
    static constexpr auto IMPORT_FN = importEmptyLastPageAppendType;
    static constexpr auto EXPORT_FN = [](xmlNodePtr node, std::string name, EmptyLastPageAppendType value) {
        return exportProp(node, name.c_str(), emptyLastPageAppendToString(value));
    };
};

template <>
struct Setting<SettingsElement::STROKE_FILTER_IGNORE_TIME> {
    using value_type = int;
    static constexpr auto XML_NAME = "strokeFilterIgnoreTime";
    static constexpr value_type DEFAULT = 150;
};

template <>
struct Setting<SettingsElement::STROKE_FILTER_IGNORE_LENGTH> {
    using value_type = double;
    static constexpr auto XML_NAME = "strokeFilterIgnoreLength";
    static constexpr value_type DEFAULT = 1.0;
};

template <>
struct Setting<SettingsElement::STROKE_FILTER_SUCCESSIVE_TIME> {
    using value_type = int;
    static constexpr auto XML_NAME = "strokeFilterSuccessiveTime";
    static constexpr value_type DEFAULT = 500;
};

template <>
struct Setting<SettingsElement::STROKE_FILTER_ENABLED> {
    using value_type = bool;
    static constexpr auto XML_NAME = "strokeFilterEnabled";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::DO_ACTION_ON_STROKE_FILTERED> {
    using value_type = bool;
    static constexpr auto XML_NAME = "doActionOnStrokeFiltered";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::TRY_SELECT_ON_STROKE_FILTERED> {
    using value_type = bool;
    static constexpr auto XML_NAME = "trySelectOnStrokeFiltered";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::LATEX_SETTINGS> {
    using value_type = LatexSettings;
    static constexpr auto XML_NAME = "latexSettings";
    static const value_type DEFAULT;
    static constexpr auto IMPORT_FN = importLatexSettings;
    static constexpr auto EXPORT_FN = exportLatexSettings;
};

template <>
struct Setting<SettingsElement::SNAP_RECOGNIZED_SHAPES> {
    using value_type = bool;
    static constexpr auto XML_NAME = "snapRecognizedShapesEnabled";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::RESTORE_LINE_WIDTH> {
    using value_type = bool;
    static constexpr auto XML_NAME = "restoreLineWidthEnabled";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::PREFERRED_LOCALE> {
    using value_type = std::string;
    static constexpr auto XML_NAME = "preferredLocale";
    static constexpr const char* DEFAULT = "";
};

template <>
struct Setting<SettingsElement::STABILIZER_AVERAGING_METHOD> {
    using value_type = StrokeStabilizer::AveragingMethod;
    static constexpr auto XML_NAME = "stabilizerAveragingMethod";
    static constexpr value_type DEFAULT = StrokeStabilizer::AveragingMethod::NONE;
    static constexpr auto IMPORT_FN = importAveragingMethod;
    static constexpr auto EXPORT_FN = [](xmlNodePtr node, std::string name, StrokeStabilizer::AveragingMethod value) {
        return exportProperty(node, name, static_cast<int>(value));
    };
    static constexpr auto VALIDATE_FN = [](value_type value) -> value_type {
        return StrokeStabilizer::isValid(value) ? value : StrokeStabilizer::AveragingMethod::NONE;
    };
};

template <>
struct Setting<SettingsElement::STABILIZER_PREPROCESSOR> {
    using value_type = StrokeStabilizer::Preprocessor;
    static constexpr auto XML_NAME = "stabilizerPreprocessor";
    static constexpr value_type DEFAULT = StrokeStabilizer::Preprocessor::NONE;
    static constexpr auto IMPORT_FN = importPreprocessor;
    static constexpr auto EXPORT_FN = [](xmlNodePtr node, std::string name, StrokeStabilizer::Preprocessor value) {
        return exportProperty(node, name, static_cast<int>(value));
    };
    static constexpr auto VALIDATE_FN = [](value_type value) -> value_type {
        return StrokeStabilizer::isValid(value) ? value : StrokeStabilizer::Preprocessor::NONE;
    };
};

template <>
struct Setting<SettingsElement::STABILIZER_BUFFERSIZE> {
    using value_type = size_t;
    static constexpr auto XML_NAME = "stabilizerBuffersize";
    static constexpr value_type DEFAULT = 20;
};

template <>
struct Setting<SettingsElement::STABILIZER_SIGMA> {
    using value_type = double;
    static constexpr auto XML_NAME = "stabilizerSigma";
    static constexpr value_type DEFAULT = 0.5;
};

template <>
struct Setting<SettingsElement::STABILIZER_DEADZONE_RADIUS> {
    using value_type = double;
    static constexpr auto XML_NAME = "stabilizerDeadzoneRadius";
    static constexpr value_type DEFAULT = 1.3;
};

template <>
struct Setting<SettingsElement::STABILIZER_DRAG> {
    using value_type = double;
    static constexpr auto XML_NAME = "stabilizerDrag";
    static constexpr value_type DEFAULT = 0.4;
};

template <>
struct Setting<SettingsElement::STABILIZER_MASS> {
    using value_type = double;
    static constexpr auto XML_NAME = "stabilizerMass";
    static constexpr value_type DEFAULT = 5.0;
};

template <>
struct Setting<SettingsElement::STABILIZER_CUSP_DETECTION> {
    using value_type = bool;
    static constexpr auto XML_NAME = "stabilizerCuspDetection";
    static constexpr value_type DEFAULT = true;
};

template <>
struct Setting<SettingsElement::STABILIZER_FINALIZE_STROKE> {
    using value_type = bool;
    static constexpr auto XML_NAME = "stabilizerFinalizeStroke";
    static constexpr value_type DEFAULT = true;
};
template <>
struct Setting<SettingsElement::USE_SPACES_AS_TAB> {
    using value_type = bool;
    static constexpr auto XML_NAME = "useSpacesForTab";
    static constexpr value_type DEFAULT = false;
};
template <>
struct Setting<SettingsElement::NUMBER_OF_SPACES_FOR_TAB> {
    using value_type = uint;
    static constexpr auto XML_NAME = "numberOfSpacesForTab";
    static constexpr value_type DEFAULT = 4;
    static constexpr auto VALIDATE_FN = [](value_type val) -> value_type { return std::min(val, 8U); };
};

// Nested Settings from here:
template <>
struct Setting<SettingsElement::NESTED_BUTTON_CONFIG> {
    using value_type = std::array<std::shared_ptr<ButtonConfig>, BUTTON_COUNT>;
    static constexpr auto XML_NAME = "buttonConfig";
    static const value_type DEFAULT;
    static constexpr auto IMPORT_FN = importButtonConfig;
    static constexpr auto EXPORT_FN = exportButtonConfig;
};

template <>
struct Setting<SettingsElement::NESTED_DEVICE_CLASSES> {
    using value_type = std::map<std::string, std::pair<InputDeviceTypeOption, GdkInputSource>>;
    static constexpr auto XML_NAME = "deviceClasses";
    static const value_type DEFAULT;
    static constexpr auto IMPORT_FN = importDeviceClasses;
    static constexpr auto EXPORT_FN = exportDeviceClasses;
};

template <>
struct Setting<SettingsElement::NESTED_TOOLS> {
    using value_type = SElement;
    static constexpr auto XML_NAME = "tools";
    static const value_type DEFAULT;
};

template <>
struct Setting<SettingsElement::NESTED_TOUCH> {
    using value_type = SElement;
    static constexpr auto XML_NAME = "touch";
    static const value_type DEFAULT;
};

template <>
struct Setting<SettingsElement::NESTED_LAST_USED_PAGE_BACKGROUND_COLOR> {
    using value_type = SElement;
    static constexpr auto XML_NAME = "lastUsedPageBgColor";
    static const value_type DEFAULT;
};


// Importer, exporter, validator and comment struct here:
template <SettingsElement e, typename U = void>
struct importer {
    static constexpr auto fn = importProperty<typename Setting<e>::value_type>;
};
template <SettingsElement e>
struct importer<e, std::void_t<decltype(Setting<e>::IMPORT_FN)>> {
    static constexpr auto fn = Setting<e>::IMPORT_FN;
};

template <SettingsElement e, typename U = void>
struct exporter {
    static constexpr auto fn = exportProperty<getter_return_t<typename Setting<e>::value_type>>;
};
template <SettingsElement e>
struct exporter<e, std::void_t<decltype(Setting<e>::EXPORT_FN)>> {
    static constexpr auto fn = Setting<e>::EXPORT_FN;
};

template <SettingsElement e, typename U = void>
struct validator {
    static constexpr auto fn = [](const typename Setting<e>::value_type& val) ->
            typename Setting<e>::value_type { return val; };
    static constexpr bool enable = false;
};
template <SettingsElement e>
struct validator<e, std::void_t<decltype(Setting<e>::VALIDATE_FN)>> {
    static constexpr auto fn = Setting<e>::VALIDATE_FN;
    static constexpr bool enable = true;
};

template <SettingsElement e, typename U = void>
struct comment {
    static constexpr auto text = nullptr;
};
template <SettingsElement e>
struct comment<e, std::void_t<decltype(Setting<e>::COMMENT)>> {
    static constexpr auto text = Setting<e>::COMMENT;
};


// Definition of map mapping property names to other properties
// If a property is renamed add a line like this '{"oldName", "newName"},'
static std::map<std::string, std::string> propertyRenamerMap = {
        {"presureSensitivity", "pressureSensitivity"},
        // Redirect all latex setting properties to latexSettings
        {"latexSettings.autoCheckDependencies", "latexSettings"},
        {"latexSettings.defaultText", "latexSettings"},
        {"latexSettings.globalTemplatePath", "latexSettings"},
        {"latexSettings.genCmd", "latexSettings"},
        {"latexSettings.sourceViewThemeId", "latexSettings"},
        {"latexSettings.editorFont", "latexSettings"},
        {"latexSettings.useCustomEditorFont", "latexSettings"},
        {"latexSettings.editorWordWrap", "latexSettings"},
        {"latexSettings.sourceViewAutoIndent", "latexSettings"},
        {"latexSettings.sourceViewSyntaxHighlight", "latexSettings"},
        {"latexSettings.sourceViewShowLineNumbers", "latexSettings"}};
