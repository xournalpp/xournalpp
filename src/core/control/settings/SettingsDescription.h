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

// TODO: improve this comment
/*
 * In order to add settings add an element to the SettingsElement enum
 */

enum class SettingsElement {
    SETTING_FONT,
    SETTING_PRESSURE_SENSITIVITY,
    SETTING_MINIMUM_PRESSURE,
    SETTING_PRESSURE_MULTIPLIER,
    SETTING_ENABLE_ZOOM_GESTURES,
    SETTING_SELECTED_TOOLBAR,
    SETTING_LAST_SAVE_PATH,
    SETTING_LAST_OPEN_PATH,
    SETTING_LAST_IMAGE_PATH,
    SETTING_EDGE_PAN_SPEED,
    SETTING_EDGE_PAN_MAX_MULT,
    SETTING_ZOOM_STEP,
    SETTING_ZOOM_STEP_SCROLL,
    SETTING_DISPLAY_DPI,
    SETTING_MAIN_WINDOW_WIDTH,
    SETTING_MAIN_WINDOW_HEIGHT,
    SETTING_MAXIMIZED,
    SETTING_SHOW_TOOLBAR,
    SETTING_SHOW_FILEPATH_IN_TITLEBAR,
    SETTING_SHOW_PAGE_NUMBER_IN_TITLEBAR,
    SETTING_SHOW_SIDEBAR,
    SETTING_SIDEBAR_NUMBERING_STYLE,
    SETTING_SIDEBAR_WIDTH,
    SETTING_SIDEBAR_ON_RIGHT,
    SETTING_SCROLLBAR_ON_LEFT,
    SETTING_MENUBAR_VISIBLE,
    SETTING_NUM_COLUMNS,
    SETTING_NUM_ROWS,
    SETTING_VIEW_FIXED_ROWS,
    SETTING_LAYOUT_VERTICAL,
    SETTING_LAYOUT_RIGHT_TO_LEFT,
    SETTING_LAYOUT_BOTTOM_TO_TOP,
    SETTING_SHOW_PAIRED_PAGES,
    SETTING_NUM_PAIRS_OFFSET,
    // SETTING_PRESENTATION_MODE, // TODO: remove
    SETTING_AUTOLOAD_MOST_RECENT,
    SETTING_AUTOLOAD_PDF_XOJ,
    SETTING_STYLUS_CURSOR_TYPE,
    SETTING_ERASER_VISIBILITY,
    SETTING_ICON_THEME,
    SETTING_HIGHLIGHT_POSITION,
    SETTING_CURSOR_HIGHLIGHT_COLOR,
    SETTING_CURSOR_HIGHLIGHT_RADIUS,
    SETTING_CURSOR_HIGHLIGHT_BORDER_COLOR,
    SETTING_CURSOR_HIGHLIGHT_BORDER_WIDTH,
    SETTING_DARK_THEME,
    SETTING_USE_STOCK_ICONS,
    SETTING_DEFAULT_SAVE_NAME,
    SETTING_DEFAULT_PDF_EXPORT_NAME,
    SETTING_PLUGIN_ENABLED,
    SETTING_PLUGIN_DISABLED,
    SETTING_PAGE_TEMPLATE,
    SETTING_SIZE_UNIT,
    SETTING_AUDIO_FOLDER,
    SETTING_AUTOSAVE_ENABLED,
    SETTING_AUTOSAVE_TIMEOUT,
    SETTING_ACTIVE_VIEW_MODE,
    SETTING_DEFAULT_VIEW_MODE_ATTRIBUTES,
    SETTING_FULLSCREEN_VIEW_MODE_ATTRIBUTES,
    SETTING_PRESENTATION_VIEW_MODE_ATTRIBUTES,
    SETTING_TOUCH_ZOOM_START_THRESHOLD,
    SETTING_PAGE_RERENDER_THRESHOLD,
    SETTING_PDF_PAGE_CACHE_SIZE,
    SETTING_PRELOAD_PAGES_BEFORE,
    SETTING_PRELOAD_PAGES_AFTER,
    SETTING_EAGER_PAGE_CLEANUP,
    SETTING_SELECTION_BORDER_COLOR,
    SETTING_SELECTION_MARKER_COLOR,
    SETTING_ACTIVE_SELECTION_COLOR,
    SETTING_BACKGROUND_COLOR,
    SETTING_ADD_HORIZONTAL_SPACE,
    SETTING_ADD_HORIZONTAL_SPACE_AMOUNT_RIGHT,
    SETTING_ADD_HORIZONTAL_SPACE_AMOUNT_LEFT,
    SETTING_ADD_VERTICAL_SPACE,
    SETTING_ADD_VERTICAL_SPACE_AMOUNT_ABOVE,
    SETTING_ADD_VERTICAL_SPACE_AMOUNT_BELOW,
    SETTING_UNLIMITED_SCROLLING,
    SETTING_DRAW_DIRECTION_MODS_ENABLE,
    SETTING_DRAW_DIRECTION_MODS_RADIUS,
    SETTING_SNAP_ROTATION,
    SETTING_SNAP_ROTATION_TOLERANCE,
    SETTING_SNAP_GRID,
    SETTING_SNAP_GRID_SIZE,
    SETTING_SNAP_GRID_TOLERANCE,
    SETTING_STROKE_RECOGNIZER_MIN_SIZE,
    SETTING_TOUCH_DRAWING,
    SETTING_GTK_TOUCH_INERTIAL_SCROLLING,
    SETTING_PRESSURE_GUESSING,
    SETTING_SCROLLBAR_HIDE_TYPE,
    SETTING_DISABLE_SCROLLBAR_FADEOUT,
    SETTING_DISABLE_AUDIO,
    SETTING_AUDIO_SAMPLE_RATE,
    SETTING_AUDIO_GAIN,
    SETTING_DEFAULT_SEEK_TIME,
    SETTING_AUDIO_INPUT_DEVICE,
    SETTING_AUDIO_OUTPUT_DEVICE,
    SETTING_NUM_IGNORED_STYLUS_EVENTS,
    SETTING_INPUT_SYSTEM_TPC_BUTTON,
    SETTING_INPUT_SYSTEM_DRAW_OUTSIDE_WINDOW,
    SETTING_EMPTY_LAST_PAGE_APPEND,
    SETTING_STROKE_FILTER_IGNORE_TIME,
    SETTING_STROKE_FILTER_IGNORE_LENGTH,
    SETTING_STROKE_FILTER_SUCCESSIVE_TIME,
    SETTING_STROKE_FILTER_ENABLED,
    SETTING_DO_ACTION_ON_STROKE_FILTERED,
    SETTING_TRY_SELECT_ON_STROKE_FILTERED,
    SETTING_LATEX_SETTINGS,
    SETTING_SNAP_RECOGNIZED_SHAPES,
    SETTING_RESTORE_LINE_WIDTH,
    SETTING_PREFERRED_LOCALE,
    SETTING_STABILIZER_AVERAGING_METHOD,
    SETTING_STABILIZER_PREPROCESSOR,
    SETTING_STABILIZER_BUFFERSIZE,
    SETTING_STABILIZER_SIGMA,
    SETTING_STABILIZER_DEADZONE_RADIUS,
    SETTING_STABILIZER_DRAG,
    SETTING_STABILIZER_MASS,
    SETTING_STABILIZER_CUSP_DETECTION,
    SETTING_STABILIZER_FINALIZE_STROKE,
    USE_SPACES_AS_TAB,
    NUMBER_OF_SPACES_FOR_TAB,
    // Nested Settings from here on
    // Saved in data tags in the settings file
    SETTING_NESTED_TOUCH,
    SETTING_NESTED_LAST_USED_PAGE_BACKGROUND_COLOR,
    SETTING_NESTED_DEVICE_CLASSES,
    SETTING_NESTED_BUTTON_CONFIG,
    SETTING_NESTED_TOOLS,
    // Don't add more entries below this comment
    ENUM_COUNT
};


// Definitions of import functions
template <typename T>
bool importProperty(xmlNodePtr node, T& var);

template <>
bool importProperty(xmlNodePtr node, std::string& var);

template <>
bool importProperty(xmlNodePtr node, fs::path& var);

template <>
bool importProperty(xmlNodePtr node, bool& var);

template <>
bool importProperty(xmlNodePtr node, double& var);

template <>
bool importProperty(xmlNodePtr node, int& var);

template <>
bool importProperty(xmlNodePtr node, uint& var);

template <>
bool importProperty(xmlNodePtr node, Color& var);

template <>
bool importProperty(xmlNodePtr node, size_t& var);

template <>
bool importProperty(xmlNodePtr node, XojFont& var);

template <>
bool importProperty(xmlNodePtr node, SidebarNumberingStyle& var);

template <>
bool importProperty(xmlNodePtr node, StylusCursorType& var);

template <>
bool importProperty(xmlNodePtr node, EraserVisibility& var);

template <>
bool importProperty(xmlNodePtr node, IconTheme& var);

template <>
bool importProperty(xmlNodePtr node, ViewMode& var);

template <>
bool importProperty(xmlNodePtr node, ScrollbarHideType& var);

template <>
bool importProperty(xmlNodePtr node, EmptyLastPageAppendType& var);

template <>
bool importProperty(xmlNodePtr node, LatexSettings& var);

template <>
bool importProperty(xmlNodePtr node, StrokeStabilizer::AveragingMethod& var);

template <>
bool importProperty(xmlNodePtr node, StrokeStabilizer::Preprocessor& var);

template <>
bool importProperty(xmlNodePtr node, std::array<std::shared_ptr<ButtonConfig>, BUTTON_COUNT>& var);

template <>
bool importProperty(xmlNodePtr node, std::map<std::string, std::pair<InputDeviceTypeOption, GdkInputSource>>& var);

template <>
bool importProperty(xmlNodePtr node, SElement& var);


// Definitions of export functions
template <typename T>
xmlNodePtr exportProperty(xmlNodePtr parent, std::string name, const T& value);

template <>
xmlNodePtr exportProperty(xmlNodePtr node, std::string name, const std::string& value);

template <>
xmlNodePtr exportProperty(xmlNodePtr node, std::string name, const fs::path& value);

template <>
xmlNodePtr exportProperty(xmlNodePtr node, std::string name, const bool& value);

template <>
xmlNodePtr exportProperty(xmlNodePtr node, std::string name, const double& value);

template <>
xmlNodePtr exportProperty(xmlNodePtr node, std::string name, const int& value);

template <>
xmlNodePtr exportProperty(xmlNodePtr node, std::string name, const uint& value);

template <>
xmlNodePtr exportProperty(xmlNodePtr node, std::string name, const Color& value);

template <>
xmlNodePtr exportProperty(xmlNodePtr node, std::string name, const size_t& value);

template <>
xmlNodePtr exportProperty(xmlNodePtr node, std::string name, const XojFont& value);

template <>
xmlNodePtr exportProperty(xmlNodePtr node, std::string name, const SidebarNumberingStyle& value);

template <>
xmlNodePtr exportProperty(xmlNodePtr node, std::string name, const StylusCursorType& value);

template <>
xmlNodePtr exportProperty(xmlNodePtr node, std::string name, const EraserVisibility& value);

template <>
xmlNodePtr exportProperty(xmlNodePtr node, std::string name, const IconTheme& value);

template <>
xmlNodePtr exportProperty(xmlNodePtr node, std::string name, const ViewMode& value);

template <>
xmlNodePtr exportProperty(xmlNodePtr node, std::string name, const ScrollbarHideType& value);

template <>
xmlNodePtr exportProperty(xmlNodePtr node, std::string name, const EmptyLastPageAppendType& value);

template <>
xmlNodePtr exportProperty(xmlNodePtr node, std::string name, const LatexSettings& value);

template <>
xmlNodePtr exportProperty(xmlNodePtr node, std::string name, const StrokeStabilizer::AveragingMethod& value);

template <>
xmlNodePtr exportProperty(xmlNodePtr node, std::string name, const StrokeStabilizer::Preprocessor& value);

template <>
xmlNodePtr exportProperty(xmlNodePtr node, std::string name,
                          const std::array<std::shared_ptr<ButtonConfig>, BUTTON_COUNT>& value);

template <>
xmlNodePtr exportProperty(xmlNodePtr node, std::string name,
                          const std::map<std::string, std::pair<InputDeviceTypeOption, GdkInputSource>>& value);

template <>
xmlNodePtr exportProperty(xmlNodePtr node, std::string name, const SElement& value);


// Definitions of validation functions
template <typename t>
t noValidate(t val) {
    return val;
}


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
struct getter_return<bool> {
    using type = bool;
};
template <>
struct getter_return<double> {
    using type = double;
};
template <>
struct getter_return<int> {
    using type = int;
};
template <>
struct getter_return<uint> {
    using type = uint;
};
template <>
struct getter_return<Color> {
    using type = Color;
};
template <>
struct getter_return<size_t> {
    using type = size_t;
};
template <>
struct getter_return<XojFont> {
    using type = const XojFont&;
};
template <>
struct getter_return<SidebarNumberingStyle> {
    using type = SidebarNumberingStyle;
};
template <>
struct getter_return<StylusCursorType> {
    using type = StylusCursorType;
};
template <>
struct getter_return<EraserVisibility> {
    using type = EraserVisibility;
};
template <>
struct getter_return<IconTheme> {
    using type = IconTheme;
};
template <>
struct getter_return<ViewMode> {
    using type = ViewMode;
};
template <>
struct getter_return<ScrollbarHideType> {
    using type = ScrollbarHideType;
};
template <>
struct getter_return<EmptyLastPageAppendType> {
    using type = EmptyLastPageAppendType;
};
template <>
struct getter_return<LatexSettings> {
    using type = const LatexSettings&;
};
template <>
struct getter_return<StrokeStabilizer::AveragingMethod> {
    using type = StrokeStabilizer::AveragingMethod;
};
template <>
struct getter_return<StrokeStabilizer::Preprocessor> {
    using type = StrokeStabilizer::Preprocessor;
};
template <>
struct getter_return<std::array<std::shared_ptr<ButtonConfig>, BUTTON_COUNT>> {
    using type = const std::array<std::shared_ptr<ButtonConfig>, BUTTON_COUNT>&;
};
template <>
struct getter_return<std::map<std::string, std::pair<InputDeviceTypeOption, GdkInputSource>>> {
    using type = const std::map<std::string, std::pair<InputDeviceTypeOption, GdkInputSource>>&;
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
 *    value_type: The type your settings value should have
 *    xmlName: The name you want for your property in the file
 *    DEFAULT: The default value of your Setting, if your value_type is not a literal type leave it unset here, and add
 * the initialization in SettingsDescriptions.cpp at the top COMMENT: The comment in the file above your setting, if not
 * needed leave nullptr for none IMPORT_FN: Function that reads your value from the xml tag EXPORT_FN: Function that
 * writes your value into an xml tag and adds it to the document root VALIDATE_FN: Function that checks if the value is
 * correct and always returns a valid value, called at import and setter
 * };
 */
// TODO: fix these comments
/* Template:
 * template<>
 * struct Setting<SettingsElement::> {
 *     using value_type = ;
 *     static constexpr auto xmlName = "";
 *     static value_type DEFAULT;
 *     static constexpr auto COMMENT = "";
 * };
 */

template <>
struct Setting<SettingsElement::SETTING_FONT> {
    using value_type = XojFont;
    static constexpr auto xmlName = "font";
    static value_type DEFAULT;
};

template <>
struct Setting<SettingsElement::SETTING_PRESSURE_SENSITIVITY> {
    using value_type = bool;
    static constexpr auto xmlName = "pressureSensitivity";
    static constexpr value_type DEFAULT = true;
};

template <>
struct Setting<SettingsElement::SETTING_MINIMUM_PRESSURE> {
    using value_type = double;
    static constexpr auto xmlName = "minimumPressure";
    static constexpr value_type DEFAULT = 0.05;
    static constexpr auto VALIDATE_FN = [](value_type val) -> value_type { return std::max<value_type>(val, 0.01); };
};

template <>
struct Setting<SettingsElement::SETTING_PRESSURE_MULTIPLIER> {
    using value_type = double;
    static constexpr auto xmlName = "pressureMultiplier";
    static constexpr value_type DEFAULT = 1.0;
};

template <>
struct Setting<SettingsElement::SETTING_ENABLE_ZOOM_GESTURES> {
    using value_type = bool;
    static constexpr auto xmlName = "zoomGesturesEnabled";
    static constexpr value_type DEFAULT = true;
};

template <>
struct Setting<SettingsElement::SETTING_SELECTED_TOOLBAR> {
    using value_type = std::string;
    static constexpr auto xmlName = "selectedToolbar";
    static constexpr const char* DEFAULT = "Portrait";
};

template <>
struct Setting<SettingsElement::SETTING_LAST_SAVE_PATH> {
    using value_type = fs::path;
    static constexpr auto xmlName = "lastSavePath";
    static constexpr const char* DEFAULT = "";
};

template <>
struct Setting<SettingsElement::SETTING_LAST_OPEN_PATH> {
    using value_type = fs::path;
    static constexpr auto xmlName = "lastOpenPath";
    static constexpr const char* DEFAULT = "";
};

template <>
struct Setting<SettingsElement::SETTING_LAST_IMAGE_PATH> {
    using value_type = fs::path;
    static constexpr auto xmlName = "lastImagePath";
    static constexpr const char* DEFAULT = "";
};

template <>
struct Setting<SettingsElement::SETTING_EDGE_PAN_SPEED> {
    using value_type = double;
    static constexpr auto xmlName = "edgePanSpeed";
    static constexpr value_type DEFAULT = 20.0;
};

template <>
struct Setting<SettingsElement::SETTING_EDGE_PAN_MAX_MULT> {
    using value_type = double;
    static constexpr auto xmlName = "edgePanMaxMult";
    static constexpr value_type DEFAULT = 5.0;
};

template <>
struct Setting<SettingsElement::SETTING_ZOOM_STEP> {
    using value_type = double;
    static constexpr auto xmlName = "zoomStep";
    static constexpr value_type DEFAULT = 10.0;
};

template <>
struct Setting<SettingsElement::SETTING_ZOOM_STEP_SCROLL> {
    using value_type = double;
    static constexpr auto xmlName = "zoomStepScroll";
    static constexpr value_type DEFAULT = 2.0;
};

template <>
struct Setting<SettingsElement::SETTING_DISPLAY_DPI> {
    using value_type = int;
    static constexpr auto xmlName = "displayDpi";
    static constexpr value_type DEFAULT = 72;
};

template <>
struct Setting<SettingsElement::SETTING_MAIN_WINDOW_WIDTH> {
    using value_type = int;
    static constexpr auto xmlName = "mainWndWidth";
    static constexpr value_type DEFAULT = 800;
};

template <>
struct Setting<SettingsElement::SETTING_MAIN_WINDOW_HEIGHT> {
    using value_type = int;
    static constexpr auto xmlName = "mainWndHeight";
    static constexpr value_type DEFAULT = 600;
};

template <>
struct Setting<SettingsElement::SETTING_MAXIMIZED> {
    using value_type = bool;
    static constexpr auto xmlName = "maximized";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::SETTING_SHOW_TOOLBAR> {
    using value_type = bool;
    static constexpr auto xmlName = "showToolbar";
    static constexpr value_type DEFAULT = true;
};

template <>
struct Setting<SettingsElement::SETTING_SHOW_FILEPATH_IN_TITLEBAR> {
    using value_type = bool;
    static constexpr auto xmlName = "filepathShownInTitlebar";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::SETTING_SHOW_PAGE_NUMBER_IN_TITLEBAR> {
    using value_type = bool;
    static constexpr auto xmlName = "pageNumberShownInTitlebar";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::SETTING_SHOW_SIDEBAR> {
    using value_type = bool;
    static constexpr auto xmlName = "showSidebar";
    static constexpr value_type DEFAULT = true;
};

template <>
struct Setting<SettingsElement::SETTING_SIDEBAR_NUMBERING_STYLE> {
    using value_type = SidebarNumberingStyle;
    static constexpr auto xmlName = "sidebarNumberingStyle";
    static constexpr value_type DEFAULT = SidebarNumberingStyle::DEFAULT;
    static constexpr auto VALIDATE_FN = [](value_type val) -> value_type {
        return std::max<value_type>(SidebarNumberingStyle::MIN, std::min<value_type>(val, SidebarNumberingStyle::MAX));
    };
};

template <>
struct Setting<SettingsElement::SETTING_SIDEBAR_WIDTH> {
    using value_type = int;
    static constexpr auto xmlName = "sidebarWidth";
    static constexpr value_type DEFAULT = 150;
    static constexpr auto VALIDATE_FN = [](value_type val) -> value_type { return std::max<value_type>(val, 50); };
};

template <>
struct Setting<SettingsElement::SETTING_SIDEBAR_ON_RIGHT> {
    using value_type = bool;
    static constexpr auto xmlName = "sidebarOnRight";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::SETTING_SCROLLBAR_ON_LEFT> {
    using value_type = bool;
    static constexpr auto xmlName = "scrollbarOnLeft";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::SETTING_MENUBAR_VISIBLE> {
    using value_type = bool;
    static constexpr auto xmlName = "menubarVisible";
    static constexpr value_type DEFAULT = true;
};

template <>
struct Setting<SettingsElement::SETTING_NUM_COLUMNS> {
    using value_type = int;
    static constexpr auto xmlName = "numColumns";
    static constexpr value_type DEFAULT = 1;
};

template <>
struct Setting<SettingsElement::SETTING_NUM_ROWS> {
    using value_type = int;
    static constexpr auto xmlName = "numRows";
    static constexpr value_type DEFAULT = 1;
};

template <>
struct Setting<SettingsElement::SETTING_VIEW_FIXED_ROWS> {
    using value_type = bool;
    static constexpr auto xmlName = "viewFixedRows";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::SETTING_LAYOUT_VERTICAL> {
    using value_type = bool;
    static constexpr auto xmlName = "layoutVertical";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::SETTING_LAYOUT_RIGHT_TO_LEFT> {
    using value_type = bool;
    static constexpr auto xmlName = "layoutRightToLeft";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::SETTING_LAYOUT_BOTTOM_TO_TOP> {
    using value_type = bool;
    static constexpr auto xmlName = "layoutBottomToTop";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::SETTING_SHOW_PAIRED_PAGES> {
    using value_type = bool;
    static constexpr auto xmlName = "showPairedPages";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::SETTING_NUM_PAIRS_OFFSET> {
    using value_type = int;
    static constexpr auto xmlName = "numPairsOffset";
    static constexpr value_type DEFAULT = 1;
};

/*template<>
struct Setting<SettingsElement::SETTING_PRESENTATION_MODE> { // TODO: remove this as it is not needed
    using value_type = bool;
    static constexpr auto xmlName = "presentationMode";
    static constexpr value_type DEFAULT = false;
};*/

template <>
struct Setting<SettingsElement::SETTING_AUTOLOAD_MOST_RECENT> {
    using value_type = bool;
    static constexpr auto xmlName = "autoloadMostRecent";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::SETTING_AUTOLOAD_PDF_XOJ> {
    using value_type = bool;
    static constexpr auto xmlName = "autoloadPdfXoj";
    static constexpr value_type DEFAULT = true;
};

template <>
struct Setting<SettingsElement::SETTING_STYLUS_CURSOR_TYPE> {
    using value_type = StylusCursorType;
    static constexpr auto xmlName = "stylusCursorType";
    static constexpr value_type DEFAULT = StylusCursorType::STYLUS_CURSOR_DOT;
    static constexpr auto COMMENT =
            "The cursor icon used with a stylus, allowed values are \"none\", \"dot\", \"big\", \"arrow\"";
};

template <>
struct Setting<SettingsElement::SETTING_ERASER_VISIBILITY> {
    using value_type = EraserVisibility;
    static constexpr auto xmlName = "eraserVisibility";
    static constexpr value_type DEFAULT = EraserVisibility::ERASER_VISIBILITY_ALWAYS;
    static constexpr auto COMMENT = "The eraser cursor visibility used with a stylus, allowed values are \"never\", "
                                    "\"always\", \"hover\", \"touch\"";
};

template <>
struct Setting<SettingsElement::SETTING_ICON_THEME> {
    using value_type = IconTheme;
    static constexpr auto xmlName = "iconTheme";
    static constexpr value_type DEFAULT = IconTheme::ICON_THEME_COLOR;
    static constexpr auto COMMENT = "The icon theme, allowed values are \"iconsColor\", \"iconsLucide\"";
};

template <>
struct Setting<SettingsElement::SETTING_HIGHLIGHT_POSITION> {
    using value_type = bool;
    static constexpr auto xmlName = "highlightPosition";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::SETTING_CURSOR_HIGHLIGHT_COLOR> {
    using value_type = Color;
    static constexpr auto xmlName = "cursorHighlightColor";
    static constexpr value_type DEFAULT = ColorU8(0x80FFFF00);
};

template <>
struct Setting<SettingsElement::SETTING_CURSOR_HIGHLIGHT_RADIUS> {
    using value_type = double;
    static constexpr auto xmlName = "cursorHighlightRadius";
    static constexpr value_type DEFAULT = 30.0;
};

template <>
struct Setting<SettingsElement::SETTING_CURSOR_HIGHLIGHT_BORDER_COLOR> {
    using value_type = Color;
    static constexpr auto xmlName = "cursorHighlightBorderColor";
    static constexpr value_type DEFAULT = ColorU8(0x800000FF);
};

template <>
struct Setting<SettingsElement::SETTING_CURSOR_HIGHLIGHT_BORDER_WIDTH> {
    using value_type = double;
    static constexpr auto xmlName = "cursorHighlightBorderWidth";
    static constexpr value_type DEFAULT = 0.0;
};

template <>
struct Setting<SettingsElement::SETTING_DARK_THEME> {
    using value_type = bool;
    static constexpr auto xmlName = "darkTheme";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::SETTING_USE_STOCK_ICONS> {
    using value_type = bool;
    static constexpr auto xmlName = "useStockIcons";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::SETTING_DEFAULT_SAVE_NAME> {
    using value_type = std::string;
    static constexpr auto xmlName = "defaultSaveName";
    static value_type DEFAULT;
};

template <>
struct Setting<SettingsElement::SETTING_DEFAULT_PDF_EXPORT_NAME> {
    using value_type = std::string;
    static constexpr auto xmlName = "defaultPdfExportName";
    static value_type DEFAULT;
};

template <>
struct Setting<SettingsElement::SETTING_PLUGIN_ENABLED> {
    using value_type = std::string;
    static constexpr auto xmlName = "pluginEnabled";
    static constexpr const char* DEFAULT = "";
};

template <>
struct Setting<SettingsElement::SETTING_PLUGIN_DISABLED> {
    using value_type = std::string;
    static constexpr auto xmlName = "pluginDisabled";
    static constexpr const char* DEFAULT = "";
};

template <>
struct Setting<SettingsElement::SETTING_PAGE_TEMPLATE> {
    using value_type = std::string;
    static constexpr auto xmlName = "pageTemplate";
    static constexpr const char* DEFAULT = "xoj/"
                                           "template\ncopyLastPageSettings=true\nsize=595.275591x841."
                                           "889764\nbackgroundType=lined\nbackgroundColor=#ffffff\n";
    static constexpr auto COMMENT = "Config for new pages";
};

template <>
struct Setting<SettingsElement::SETTING_SIZE_UNIT> {
    using value_type = std::string;
    static constexpr auto xmlName = "sizeUnit";
    static constexpr const char* DEFAULT = "";
};

template <>
struct Setting<SettingsElement::SETTING_AUDIO_FOLDER> {
    using value_type = fs::path;
    static constexpr auto xmlName = "audioFolder";
    static constexpr const char* DEFAULT = "";
};

template <>
struct Setting<SettingsElement::SETTING_AUTOSAVE_ENABLED> {
    using value_type = bool;
    static constexpr auto xmlName = "autosaveEnabled";
    static constexpr value_type DEFAULT = true;
};

template <>
struct Setting<SettingsElement::SETTING_AUTOSAVE_TIMEOUT> {
    using value_type = int;
    static constexpr auto xmlName = "autosaveTimeout";
    static constexpr value_type DEFAULT = 3;
};

template <>
struct Setting<SettingsElement::SETTING_ACTIVE_VIEW_MODE> {
    using value_type = ViewModeId;
    static constexpr auto xmlName = "";  // This setting is not saved to the config file
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
struct Setting<SettingsElement::SETTING_DEFAULT_VIEW_MODE_ATTRIBUTES> {
    using value_type = ViewMode;
    static constexpr auto xmlName = "defaultViewModeAttributes";
    static constexpr value_type DEFAULT = VIEW_MODE_STRUCT_DEFAULT;
    static constexpr auto COMMENT = "Which GUI elements are shown in default view mode, separated by a colon (,)";
};

template <>
struct Setting<SettingsElement::SETTING_FULLSCREEN_VIEW_MODE_ATTRIBUTES> {
    using value_type = ViewMode;
    static constexpr auto xmlName = "fullscreenViewModeAttributes";
    static constexpr value_type DEFAULT = VIEW_MODE_STRUCT_FULLSCREEN;
    static constexpr auto COMMENT = "Which GUI elements are shown in fullscreen view mode, separated by a colon (,)";
};

template <>
struct Setting<SettingsElement::SETTING_PRESENTATION_VIEW_MODE_ATTRIBUTES> {
    using value_type = ViewMode;
    static constexpr auto xmlName = "presentationViewModeAttributes";
    static constexpr value_type DEFAULT = VIEW_MODE_STRUCT_PRESENTATION;
    static constexpr auto COMMENT = "Which GUI elements are shown in presentation view mode, separated by a colon (,)";
};

template <>
struct Setting<SettingsElement::SETTING_TOUCH_ZOOM_START_THRESHOLD> {
    using value_type = double;
    static constexpr auto xmlName = "touchZoomStartThreshold";
    static constexpr value_type DEFAULT = 0.0;
};

template <>
struct Setting<SettingsElement::SETTING_PAGE_RERENDER_THRESHOLD> {
    using value_type = double;
    static constexpr auto xmlName = "pageRerenderThreshold";
    static constexpr value_type DEFAULT = 5.0;
};

template <>
struct Setting<SettingsElement::SETTING_PDF_PAGE_CACHE_SIZE> {
    using value_type = int;
    static constexpr auto xmlName = "pdfPageCacheSize";
    static constexpr value_type DEFAULT = 10;
    static constexpr auto COMMENT = "The count of rendered PDF pages which will be cached.";
};

template <>
struct Setting<SettingsElement::SETTING_PRELOAD_PAGES_BEFORE> {
    using value_type = uint;
    static constexpr auto xmlName = "preloadPagesBefore";
    static constexpr value_type DEFAULT = 3;
};

template <>
struct Setting<SettingsElement::SETTING_PRELOAD_PAGES_AFTER> {
    using value_type = uint;
    static constexpr auto xmlName = "preloadPagesAfter";
    static constexpr value_type DEFAULT = 5;
};

template <>
struct Setting<SettingsElement::SETTING_EAGER_PAGE_CLEANUP> {
    using value_type = bool;
    static constexpr auto xmlName = "eagerPageCleanup";
    static constexpr value_type DEFAULT = true;
};

template <>
struct Setting<SettingsElement::SETTING_SELECTION_BORDER_COLOR> {
    using value_type = Color;
    static constexpr auto xmlName = "selectionBorderColor";
    static constexpr value_type DEFAULT = Colors::red;
};

template <>
struct Setting<SettingsElement::SETTING_SELECTION_MARKER_COLOR> {
    using value_type = Color;
    static constexpr auto xmlName = "selectionMarkerColor";
    static constexpr value_type DEFAULT = Colors::xopp_cornflowerblue;
};

template <>
struct Setting<SettingsElement::SETTING_ACTIVE_SELECTION_COLOR> {
    using value_type = Color;
    static constexpr auto xmlName = "activeSelectionColor";
    static constexpr value_type DEFAULT = Colors::lawngreen;
};

template <>
struct Setting<SettingsElement::SETTING_BACKGROUND_COLOR> {
    using value_type = Color;
    static constexpr auto xmlName = "backgroundColor";
    static constexpr value_type DEFAULT = Colors::xopp_gainsboro02;
};

template <>
struct Setting<SettingsElement::SETTING_ADD_HORIZONTAL_SPACE> {
    using value_type = bool;
    static constexpr auto xmlName = "addHorizontalSpace";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::SETTING_ADD_HORIZONTAL_SPACE_AMOUNT_RIGHT> {
    using value_type = int;
    static constexpr auto xmlName = "addHorizontalSpaceAmountRight";
    static constexpr value_type DEFAULT = 150;
};

template <>
struct Setting<SettingsElement::SETTING_ADD_HORIZONTAL_SPACE_AMOUNT_LEFT> {
    using value_type = int;
    static constexpr auto xmlName = "addHorizontalSpaceAmountLeft";
    static constexpr value_type DEFAULT = 150;
};

template <>
struct Setting<SettingsElement::SETTING_ADD_VERTICAL_SPACE> {
    using value_type = bool;
    static constexpr auto xmlName = "addVerticalSpace";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::SETTING_ADD_VERTICAL_SPACE_AMOUNT_ABOVE> {
    using value_type = int;
    static constexpr auto xmlName = "addVerticalSpaceAmountAbove";
    static constexpr value_type DEFAULT = 150;
};

template <>
struct Setting<SettingsElement::SETTING_ADD_VERTICAL_SPACE_AMOUNT_BELOW> {
    using value_type = int;
    static constexpr auto xmlName = "addVerticalSpaceAmountBelow";
    static constexpr value_type DEFAULT = 150;
};

template <>
struct Setting<SettingsElement::SETTING_UNLIMITED_SCROLLING> {
    using value_type = bool;
    static constexpr auto xmlName = "unlimitedScrolling";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::SETTING_DRAW_DIRECTION_MODS_ENABLE> {
    using value_type = bool;
    static constexpr auto xmlName = "drawDirModsEnabled";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::SETTING_DRAW_DIRECTION_MODS_RADIUS> {
    using value_type = int;
    static constexpr auto xmlName = "drawDirModsRadius";
    static constexpr value_type DEFAULT = 50;
};

template <>
struct Setting<SettingsElement::SETTING_SNAP_ROTATION> {
    using value_type = bool;
    static constexpr auto xmlName = "snapRotation";
    static constexpr value_type DEFAULT = true;
};

template <>
struct Setting<SettingsElement::SETTING_SNAP_ROTATION_TOLERANCE> {
    using value_type = double;
    static constexpr auto xmlName = "snapRotationTolerance";
    static constexpr value_type DEFAULT = 0.3;
};

template <>
struct Setting<SettingsElement::SETTING_SNAP_GRID> {
    using value_type = bool;
    static constexpr auto xmlName = "snapGrid";
    static constexpr value_type DEFAULT = true;
};

template <>
struct Setting<SettingsElement::SETTING_SNAP_GRID_SIZE> {
    using value_type = double;
    static constexpr auto xmlName = "snapGridSize";
    static constexpr value_type DEFAULT = 14.17;
};

template <>
struct Setting<SettingsElement::SETTING_SNAP_GRID_TOLERANCE> {
    using value_type = double;
    static constexpr auto xmlName = "snapGridTolerance";
    static constexpr value_type DEFAULT = 0.50;
};

template <>
struct Setting<SettingsElement::SETTING_STROKE_RECOGNIZER_MIN_SIZE> {
    using value_type = double;
    static constexpr auto xmlName = "strokeRecognizerMinSize";
    static constexpr value_type DEFAULT = 40.0;
};

template <>
struct Setting<SettingsElement::SETTING_TOUCH_DRAWING> {
    using value_type = bool;
    static constexpr auto xmlName = "touchDrawing";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::SETTING_GTK_TOUCH_INERTIAL_SCROLLING> {
    using value_type = bool;
    static constexpr auto xmlName = "gtkTouchInertialScrolling";
    static constexpr value_type DEFAULT = true;
};

template <>
struct Setting<SettingsElement::SETTING_PRESSURE_GUESSING> {
    using value_type = bool;
    static constexpr auto xmlName = "pressureGuessing";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::SETTING_SCROLLBAR_HIDE_TYPE> {
    using value_type = ScrollbarHideType;
    static constexpr auto xmlName = "scrollbarHideType";
    static constexpr value_type DEFAULT = ScrollbarHideType::SCROLLBAR_HIDE_NONE;
    static constexpr auto COMMENT =
            "Hides scroolbars in the main window, allowed values: \"none\", \"horizontal\", \"vertical\", \"both\"";
};

template <>
struct Setting<SettingsElement::SETTING_DISABLE_SCROLLBAR_FADEOUT> {
    using value_type = bool;
    static constexpr auto xmlName = "disableScrollbarFadeout";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::SETTING_DISABLE_AUDIO> {
    using value_type = bool;
    static constexpr auto xmlName = "disableAudio";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::SETTING_AUDIO_SAMPLE_RATE> {
    using value_type = double;
    static constexpr auto xmlName = "audioSampleRate";
    static constexpr value_type DEFAULT = 44100.0;
};

template <>
struct Setting<SettingsElement::SETTING_AUDIO_GAIN> {
    using value_type = double;
    static constexpr auto xmlName = "audioGain";
    static constexpr value_type DEFAULT = 1.0;
};

template <>
struct Setting<SettingsElement::SETTING_DEFAULT_SEEK_TIME> {
    using value_type = uint;
    static constexpr auto xmlName = "defaultSeekTime";
    static constexpr value_type DEFAULT = 5;
};

template <>
struct Setting<SettingsElement::SETTING_AUDIO_INPUT_DEVICE> {
    using value_type = PaDeviceIndex;
    static constexpr auto xmlName = "audioInputDevice";
    static constexpr value_type DEFAULT = -1;  // Value formerly in AUDIO_INPUT_SYSTEM_DEFAULT
};

template <>
struct Setting<SettingsElement::SETTING_AUDIO_OUTPUT_DEVICE> {
    using value_type = PaDeviceIndex;
    static constexpr auto xmlName = "audioOutputDevice";
    static constexpr value_type DEFAULT = -1;  // Value formerly in AUDIO_OUTPUT_SYSTEM_DEFAULT
};

template <>
struct Setting<SettingsElement::SETTING_NUM_IGNORED_STYLUS_EVENTS> {
    using value_type = int;
    static constexpr auto xmlName = "numIgnoredStylusEvents";
    static constexpr value_type DEFAULT = 0;
    static constexpr auto VALIDATE_FN = [](value_type val) -> value_type { return std::max<value_type>(val, 50); };
};

template <>
struct Setting<SettingsElement::SETTING_INPUT_SYSTEM_TPC_BUTTON> {
    using value_type = bool;
    static constexpr auto xmlName = "inputSystemTPCButton";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::SETTING_INPUT_SYSTEM_DRAW_OUTSIDE_WINDOW> {
    using value_type = bool;
    static constexpr auto xmlName = "inputSystemDrawOutsideWindow";
    static constexpr value_type DEFAULT = true;
};

template <>
struct Setting<SettingsElement::SETTING_EMPTY_LAST_PAGE_APPEND> {
    using value_type = EmptyLastPageAppendType;
    static constexpr auto xmlName = "emptyLastPageAppend";
    static constexpr value_type DEFAULT = EmptyLastPageAppendType::Disabled;
    static constexpr auto COMMENT = "empty Last Page Append Type, allowed values are \"disabled\", "
                                    "\"onDrawOfLastPage\", and \"onScrollOfLastPage\"";
};

template <>
struct Setting<SettingsElement::SETTING_STROKE_FILTER_IGNORE_TIME> {
    using value_type = int;
    static constexpr auto xmlName = "strokeFilterIgnoreTime";
    static constexpr value_type DEFAULT = 150;
};

template <>
struct Setting<SettingsElement::SETTING_STROKE_FILTER_IGNORE_LENGTH> {
    using value_type = double;
    static constexpr auto xmlName = "strokeFilterIgnoreLength";
    static constexpr value_type DEFAULT = 1.0;
};

template <>
struct Setting<SettingsElement::SETTING_STROKE_FILTER_SUCCESSIVE_TIME> {
    using value_type = int;
    static constexpr auto xmlName = "strokeFilterSuccessiveTime";
    static constexpr value_type DEFAULT = 500;
};

template <>
struct Setting<SettingsElement::SETTING_STROKE_FILTER_ENABLED> {
    using value_type = bool;
    static constexpr auto xmlName = "strokeFilterEnabled";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::SETTING_DO_ACTION_ON_STROKE_FILTERED> {
    using value_type = bool;
    static constexpr auto xmlName = "doActionOnStrokeFiltered";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::SETTING_TRY_SELECT_ON_STROKE_FILTERED> {
    using value_type = bool;
    static constexpr auto xmlName = "trySelectOnStrokeFiltered";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::SETTING_LATEX_SETTINGS> {
    using value_type = LatexSettings;
    static constexpr auto xmlName = "latexSettings";
    static value_type DEFAULT;
};

template <>
struct Setting<SettingsElement::SETTING_SNAP_RECOGNIZED_SHAPES> {
    using value_type = bool;
    static constexpr auto xmlName = "snapRecognizedShapesEnabled";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::SETTING_RESTORE_LINE_WIDTH> {
    using value_type = bool;
    static constexpr auto xmlName = "restoreLineWidthEnabled";
    static constexpr value_type DEFAULT = false;
};

template <>
struct Setting<SettingsElement::SETTING_PREFERRED_LOCALE> {
    using value_type = std::string;
    static constexpr auto xmlName = "preferredLocale";
    static constexpr const char* DEFAULT = "";
};

template <>
struct Setting<SettingsElement::SETTING_STABILIZER_AVERAGING_METHOD> {
    using value_type = StrokeStabilizer::AveragingMethod;
    static constexpr auto xmlName = "stabilizerAveragingMethod";
    static constexpr value_type DEFAULT = StrokeStabilizer::AveragingMethod::NONE;
    static constexpr auto VALIDATE_FN = [](value_type value) -> value_type {
        return StrokeStabilizer::isValid(value) ? value : StrokeStabilizer::AveragingMethod::NONE;
    };
};

template <>
struct Setting<SettingsElement::SETTING_STABILIZER_PREPROCESSOR> {
    using value_type = StrokeStabilizer::Preprocessor;
    static constexpr auto xmlName = "stabilizerPreprocessor";
    static constexpr value_type DEFAULT = StrokeStabilizer::Preprocessor::NONE;
    static constexpr auto VALIDATE_FN = [](value_type value) -> value_type {
        return StrokeStabilizer::isValid(value) ? value : StrokeStabilizer::Preprocessor::NONE;
    };
};

template <>
struct Setting<SettingsElement::SETTING_STABILIZER_BUFFERSIZE> {
    using value_type = size_t;
    static constexpr auto xmlName = "stabilizerBuffersize";
    static constexpr value_type DEFAULT = 20;
};

template <>
struct Setting<SettingsElement::SETTING_STABILIZER_SIGMA> {
    using value_type = double;
    static constexpr auto xmlName = "stabilizerSigma";
    static constexpr value_type DEFAULT = 0.5;
};

template <>
struct Setting<SettingsElement::SETTING_STABILIZER_DEADZONE_RADIUS> {
    using value_type = double;
    static constexpr auto xmlName = "stabilizerDeadzoneRadius";
    static constexpr value_type DEFAULT = 1.3;
};

template <>
struct Setting<SettingsElement::SETTING_STABILIZER_DRAG> {
    using value_type = double;
    static constexpr auto xmlName = "stabilizerDrag";
    static constexpr value_type DEFAULT = 0.4;
};

template <>
struct Setting<SettingsElement::SETTING_STABILIZER_MASS> {
    using value_type = double;
    static constexpr auto xmlName = "stabilizerMass";
    static constexpr value_type DEFAULT = 5.0;
};

template <>
struct Setting<SettingsElement::SETTING_STABILIZER_CUSP_DETECTION> {
    using value_type = bool;
    static constexpr auto xmlName = "stabilizerCuspDetection";
    static constexpr value_type DEFAULT = true;
};

template <>
struct Setting<SettingsElement::SETTING_STABILIZER_FINALIZE_STROKE> {
    using value_type = bool;
    static constexpr auto xmlName = "stabilizerFinalizeStroke";
    static constexpr value_type DEFAULT = true;
};
template <>
struct Setting<SettingsElement::USE_SPACES_AS_TAB> {
    using value_type = bool;
    static constexpr auto xmlName = "useSpacesForTab";
    static constexpr value_type DEFAULT = false;
    static constexpr auto COMMENT = nullptr;
    static constexpr auto VALIDATE_FN = noValidate<value_type>;
};
template <>
struct Setting<SettingsElement::NUMBER_OF_SPACES_FOR_TAB> {
    using value_type = uint;
    static constexpr auto xmlName = "numberOfSpacesForTab";
    static constexpr value_type DEFAULT = 4;
    static constexpr auto COMMENT = nullptr;
    static constexpr auto VALIDATE_FN = [](value_type val) -> value_type { return std::min(val, 8U); };
};

// Nested Settings from here:
template <>
struct Setting<SettingsElement::SETTING_NESTED_BUTTON_CONFIG> {
    using value_type = std::array<std::shared_ptr<ButtonConfig>, BUTTON_COUNT>;
    static constexpr auto xmlName = "buttonConfig";
    static value_type DEFAULT;
};

template <>
struct Setting<SettingsElement::SETTING_NESTED_DEVICE_CLASSES> {
    using value_type = std::map<std::string, std::pair<InputDeviceTypeOption, GdkInputSource>>;
    static constexpr auto xmlName = "deviceClasses";
    static value_type DEFAULT;
};

template <>
struct Setting<SettingsElement::SETTING_NESTED_TOOLS> {
    using value_type = SElement;
    static constexpr auto xmlName = "tools";
    static value_type DEFAULT;
};

template <>
struct Setting<SettingsElement::SETTING_NESTED_TOUCH> {
    using value_type = SElement;
    static constexpr auto xmlName = "touch";
    static value_type DEFAULT;
};

template <>
struct Setting<SettingsElement::SETTING_NESTED_LAST_USED_PAGE_BACKGROUND_COLOR> {
    using value_type = SElement;
    static constexpr auto xmlName = "lastUsedPageBgColor";
    static value_type DEFAULT;
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
    static constexpr auto fn = exportProperty<typename Setting<e>::value_type>;
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
