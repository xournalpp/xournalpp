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
#include "util/PathUtil.h"    // for getConfigFile
#include "util/Util.h"        // for PRECISION_FORMAT_...
#include "util/i18n.h"        // for _
#include "util/safe_casts.h"  // for as_unsigned

#include "ButtonConfig.h"  // for ButtonConfig
#include "config-dev.h"    // for PALETTE_FILE
#include "filesystem.h"    // for path, u8path, exists


using std::string;

Settings::Settings(fs::path filepath): filepath(std::move(filepath)) {}

Settings::~Settings() = default;

auto Settings::load() -> bool {
    xmlKeepBlanksDefault(0);

    if (!fs::exists(filepath)) {
        g_warning("Settings file %s does not exist. Regenerating. ", filepath.string().c_str());
        save();
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
        if (cur->type != XML_COMMENT_NODE) {
            xmlChar* name = xmlGetProp(cur, reinterpret_cast<const xmlChar*>("name"));
            if (name == nullptr) {
                g_warning("Settings::load %s:No name property!\n", cur->name);
                continue;
            }
            std::string nameString = reinterpret_cast<const char*>(name);
            xmlFree(name);

            // Check if the current property name has an redirected name
            if (propertyRenamerMap.find(nameString) != propertyRenamerMap.end()) {
                nameString = propertyRenamerMap[nameString];
            }

            if (settings.importFunctions.find(nameString) != settings.importFunctions.end()) {
                settings.importFunctions[nameString](cur);
            } else {
                g_warning("Settings::load: No importer function found for '%s'", nameString.c_str());
            }
        }
        cur = cur->next;
    }

    xmlFreeDoc(doc);

    // This must be done before the color palette to ensure the color names are translated properly
#ifdef _WIN32
    _putenv_s("LANGUAGE", get<SettingsElement::PREFERRED_LOCALE>().c_str());
#else
    setenv("LANGUAGE", get<SettingsElement::PREFERRED_LOCALE>().c_str(), 1);
#endif

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

    /* Create metadata root */
    root = xmlNewDocNode(doc, nullptr, reinterpret_cast<const xmlChar*>("settings"), nullptr);
    xmlDocSetRootElement(doc, root);
    xmlNodePtr com = xmlNewComment(
            reinterpret_cast<const xmlChar*>("The Xournal++ settings file. Do not edit this file! "
                                             "Most settings are available in the Settings dialog, "
                                             "the others are commented in this file, but handle with care!"));
    xmlAddPrevSibling(root, com);

    for (auto exportFunction: settings.exportFunctions) { xmlNode = exportFunction(root); }

    xmlSaveFormatFileEnc(filepath.u8string().c_str(), doc, "UTF-8", 1);
    xmlFreeDoc(doc);
}


auto Settings::loadViewMode(ViewModeId mode) -> bool {
    if (mode < 0 || mode >= getViewModes().size()) {
        return false;
    }
    auto viewMode = getViewModes().at(mode);
    set<SettingsElement::MENUBAR_VISIBLE>(viewMode.showMenubar);
    set<SettingsElement::SHOW_TOOLBAR>(viewMode.showToolbar);
    set<SettingsElement::SHOW_SIDEBAR>(viewMode.showSidebar);
    set<SettingsElement::ACTIVE_VIEW_MODE>(mode);
    return true;
}

auto Settings::getViewModes() const -> std::vector<ViewMode> {
    return {get<SettingsElement::DEFAULT_VIEW_MODE_ATTRIBUTES>(),
            get<SettingsElement::FULLSCREEN_VIEW_MODE_ATTRIBUTES>(),
            get<SettingsElement::PRESENTATION_VIEW_MODE_ATTRIBUTES>()};
}

auto Settings::getActiveViewMode() const -> ViewModeId { return get<SettingsElement::ACTIVE_VIEW_MODE>(); }

// Getter- / Setter
auto Settings::isPressureSensitivity() const -> bool { return get<SettingsElement::PRESSURE_SENSITIVITY>(); }

auto Settings::isZoomGesturesEnabled() const -> bool { return get<SettingsElement::ENABLE_ZOOM_GESTURES>(); }

void Settings::setZoomGesturesEnabled(bool enable) { set<SettingsElement::ENABLE_ZOOM_GESTURES>(enable); }

auto Settings::isSidebarOnRight() const -> bool { return get<SettingsElement::SIDEBAR_ON_RIGHT>(); }

void Settings::setSidebarOnRight(bool right) { set<SettingsElement::SIDEBAR_ON_RIGHT>(right); }

auto Settings::isScrollbarOnLeft() const -> bool { return get<SettingsElement::SCROLLBAR_ON_LEFT>(); }

void Settings::setScrollbarOnLeft(bool right) { set<SettingsElement::SCROLLBAR_ON_LEFT>(right); }

auto Settings::isMenubarVisible() const -> bool { return get<SettingsElement::MENUBAR_VISIBLE>(); }

void Settings::setMenubarVisible(bool visible) { set<SettingsElement::MENUBAR_VISIBLE>(visible); }

const bool Settings::isFilepathInTitlebarShown() const { return get<SettingsElement::SHOW_FILEPATH_IN_TITLEBAR>(); }

void Settings::setFilepathInTitlebarShown(const bool shown) { set<SettingsElement::SHOW_FILEPATH_IN_TITLEBAR>(shown); }

const bool Settings::isPageNumberInTitlebarShown() const {
    return get<SettingsElement::SHOW_PAGE_NUMBER_IN_TITLEBAR>();
}

void Settings::setPageNumberInTitlebarShown(const bool shown) {
    set<SettingsElement::SHOW_PAGE_NUMBER_IN_TITLEBAR>(shown);
}

auto Settings::getAutosaveTimeout() const -> int { return get<SettingsElement::AUTOSAVE_TIMEOUT>(); }

void Settings::setAutosaveTimeout(int autosave) { set<SettingsElement::AUTOSAVE_TIMEOUT>(autosave); }

auto Settings::isAutosaveEnabled() const -> bool { return get<SettingsElement::AUTOSAVE_ENABLED>(); }

void Settings::setAutosaveEnabled(bool autosave) { set<SettingsElement::AUTOSAVE_ENABLED>(autosave); }

auto Settings::getAddVerticalSpace() const -> bool { return get<SettingsElement::ADD_VERTICAL_SPACE>(); }

void Settings::setAddVerticalSpace(bool space) { set<SettingsElement::ADD_VERTICAL_SPACE>(space); }

auto Settings::getAddVerticalSpaceAmountAbove() const -> int {
    return get<SettingsElement::ADD_VERTICAL_SPACE_AMOUNT_ABOVE>();
}

void Settings::setAddVerticalSpaceAmountAbove(int pixels) {
    set<SettingsElement::ADD_VERTICAL_SPACE_AMOUNT_ABOVE>(pixels);
}

auto Settings::getAddVerticalSpaceAmountBelow() const -> int {
    return get<SettingsElement::ADD_VERTICAL_SPACE_AMOUNT_BELOW>();
}

void Settings::setAddVerticalSpaceAmountBelow(int pixels) {
    set<SettingsElement::ADD_VERTICAL_SPACE_AMOUNT_BELOW>(pixels);
}


auto Settings::getAddHorizontalSpace() const -> bool { return get<SettingsElement::ADD_HORIZONTAL_SPACE>(); }

void Settings::setAddHorizontalSpace(bool space) { set<SettingsElement::ADD_HORIZONTAL_SPACE>(space); }

auto Settings::getAddHorizontalSpaceAmountRight() const -> int {
    return get<SettingsElement::ADD_HORIZONTAL_SPACE_AMOUNT_RIGHT>();
}

void Settings::setAddHorizontalSpaceAmountRight(int pixels) {
    set<SettingsElement::ADD_HORIZONTAL_SPACE_AMOUNT_RIGHT>(pixels);
}

auto Settings::getAddHorizontalSpaceAmountLeft() const -> int {
    return get<SettingsElement::ADD_HORIZONTAL_SPACE_AMOUNT_LEFT>();
}

void Settings::setAddHorizontalSpaceAmountLeft(int pixels) {
    set<SettingsElement::ADD_HORIZONTAL_SPACE_AMOUNT_LEFT>(pixels);
}

auto Settings::getUnlimitedScrolling() const -> bool { return get<SettingsElement::UNLIMITED_SCROLLING>(); }

void Settings::setUnlimitedScrolling(bool enable) { set<SettingsElement::UNLIMITED_SCROLLING>(enable); }

auto Settings::getDrawDirModsEnabled() const -> bool { return get<SettingsElement::DRAW_DIRECTION_MODS_ENABLE>(); }

void Settings::setDrawDirModsEnabled(bool enable) { set<SettingsElement::DRAW_DIRECTION_MODS_ENABLE>(enable); }

auto Settings::getDrawDirModsRadius() const -> int { return get<SettingsElement::DRAW_DIRECTION_MODS_RADIUS>(); }

void Settings::setDrawDirModsRadius(int pixels) { set<SettingsElement::DRAW_DIRECTION_MODS_RADIUS>(pixels); }

auto Settings::getStylusCursorType() const -> StylusCursorType { return get<SettingsElement::STYLUS_CURSOR_TYPE>(); }

void Settings::setStylusCursorType(StylusCursorType type) { set<SettingsElement::STYLUS_CURSOR_TYPE>(type); }

auto Settings::getEraserVisibility() const -> EraserVisibility { return get<SettingsElement::ERASER_VISIBILITY>(); }

void Settings::setEraserVisibility(EraserVisibility eraserVisibility) {
    set<SettingsElement::ERASER_VISIBILITY>(eraserVisibility);
}

auto Settings::getIconTheme() const -> IconTheme { return get<SettingsElement::ICON_THEME>(); }

void Settings::setIconTheme(IconTheme iconTheme) { set<SettingsElement::ICON_THEME>(iconTheme); }

auto Settings::getSidebarNumberingStyle() const -> SidebarNumberingStyle {
    return get<SettingsElement::SIDEBAR_NUMBERING_STYLE>();
};

void Settings::setSidebarNumberingStyle(SidebarNumberingStyle numberingStyle) {
    set<SettingsElement::SIDEBAR_NUMBERING_STYLE>(numberingStyle);
}

auto Settings::isHighlightPosition() const -> bool { return get<SettingsElement::HIGHLIGHT_POSITION>(); }

void Settings::setHighlightPosition(bool highlight) { set<SettingsElement::HIGHLIGHT_POSITION>(highlight); }

auto Settings::getCursorHighlightColor() const -> Color { return get<SettingsElement::CURSOR_HIGHLIGHT_COLOR>(); }

void Settings::setCursorHighlightColor(Color color) { set<SettingsElement::CURSOR_HIGHLIGHT_COLOR>(color); }

auto Settings::getCursorHighlightRadius() const -> double { return get<SettingsElement::CURSOR_HIGHLIGHT_RADIUS>(); }

void Settings::setCursorHighlightRadius(double radius) { set<SettingsElement::CURSOR_HIGHLIGHT_RADIUS>(radius); }

auto Settings::getCursorHighlightBorderColor() const -> Color {
    return get<SettingsElement::CURSOR_HIGHLIGHT_BORDER_COLOR>();
}

void Settings::setCursorHighlightBorderColor(Color color) {
    set<SettingsElement::CURSOR_HIGHLIGHT_BORDER_COLOR>(color);
}

auto Settings::getCursorHighlightBorderWidth() const -> double {
    return get<SettingsElement::CURSOR_HIGHLIGHT_BORDER_WIDTH>();
}

void Settings::setCursorHighlightBorderWidth(double radius) {
    set<SettingsElement::CURSOR_HIGHLIGHT_BORDER_WIDTH>(radius);
}

auto Settings::isSnapRotation() const -> bool { return get<SettingsElement::SNAP_ROTATION>(); }

void Settings::setSnapRotation(bool b) { set<SettingsElement::SNAP_ROTATION>(b); }

auto Settings::getSnapRotationTolerance() const -> double { return get<SettingsElement::SNAP_ROTATION_TOLERANCE>(); }

void Settings::setSnapRotationTolerance(double tolerance) { set<SettingsElement::SNAP_ROTATION_TOLERANCE>(tolerance); }

auto Settings::isSnapGrid() const -> bool { return get<SettingsElement::SNAP_GRID>(); }

void Settings::setSnapGrid(bool b) { set<SettingsElement::SNAP_GRID>(b); }

void Settings::setSnapGridTolerance(double tolerance) { set<SettingsElement::SNAP_GRID_TOLERANCE>(tolerance); }

auto Settings::getSnapGridTolerance() const -> double { return get<SettingsElement::SNAP_GRID_TOLERANCE>(); }
auto Settings::getSnapGridSize() const -> double { return get<SettingsElement::SNAP_GRID_SIZE>(); };
void Settings::setSnapGridSize(double gridSize) { set<SettingsElement::SNAP_GRID_SIZE>(gridSize); }

auto Settings::getStrokeRecognizerMinSize() const -> double {
    return get<SettingsElement::STROKE_RECOGNIZER_MIN_SIZE>();
};
void Settings::setStrokeRecognizerMinSize(double value) { set<SettingsElement::STROKE_RECOGNIZER_MIN_SIZE>(value); }

auto Settings::getTouchDrawingEnabled() const -> bool { return get<SettingsElement::TOUCH_DRAWING>(); }

void Settings::setTouchDrawingEnabled(bool b) { set<SettingsElement::TOUCH_DRAWING>(b); }

auto Settings::getGtkTouchInertialScrollingEnabled() const -> bool {
    return get<SettingsElement::GTK_TOUCH_INERTIAL_SCROLLING>();
};

void Settings::setGtkTouchInertialScrollingEnabled(bool b) { set<SettingsElement::GTK_TOUCH_INERTIAL_SCROLLING>(b); }

auto Settings::isPressureGuessingEnabled() const -> bool { return get<SettingsElement::PRESSURE_GUESSING>(); }
void Settings::setPressureGuessingEnabled(bool b) { set<SettingsElement::PRESSURE_GUESSING>(b); }

double Settings::getMinimumPressure() const { return get<SettingsElement::MINIMUM_PRESSURE>(); }
void Settings::setMinimumPressure(double minimumPressure) { set<SettingsElement::MINIMUM_PRESSURE>(minimumPressure); }

double Settings::getPressureMultiplier() const { return get<SettingsElement::PRESSURE_MULTIPLIER>(); }
void Settings::setPressureMultiplier(double multiplier) { set<SettingsElement::PRESSURE_MULTIPLIER>(multiplier); }

auto Settings::getScrollbarHideType() const -> ScrollbarHideType { return get<SettingsElement::SCROLLBAR_HIDE_TYPE>(); }

void Settings::setScrollbarHideType(ScrollbarHideType type) { set<SettingsElement::SCROLLBAR_HIDE_TYPE>(type); }

auto Settings::isAutoloadMostRecent() const -> bool { return get<SettingsElement::AUTOLOAD_MOST_RECENT>(); }

void Settings::setAutoloadMostRecent(bool load) { set<SettingsElement::AUTOLOAD_MOST_RECENT>(load); }

auto Settings::isAutoloadPdfXoj() const -> bool { return get<SettingsElement::AUTOLOAD_PDF_XOJ>(); }

void Settings::setAutoloadPdfXoj(bool load) { set<SettingsElement::AUTOLOAD_PDF_XOJ>(load); }

auto Settings::getDefaultSaveName() const -> string const& { return get<SettingsElement::DEFAULT_SAVE_NAME>(); }

void Settings::setDefaultSaveName(const string& name) { set<SettingsElement::DEFAULT_SAVE_NAME>(name); }

auto Settings::getDefaultPdfExportName() const -> string const& {
    return get<SettingsElement::DEFAULT_PDF_EXPORT_NAME>();
}

void Settings::setDefaultPdfExportName(const string& name) { set<SettingsElement::DEFAULT_PDF_EXPORT_NAME>(name); }

auto Settings::getPageTemplate() const -> string const& { return get<SettingsElement::PAGE_TEMPLATE>(); }

void Settings::setPageTemplate(const string& pageTemplate) { set<SettingsElement::PAGE_TEMPLATE>(pageTemplate); }

auto Settings::getAudioFolder() const -> fs::path const& { return get<SettingsElement::AUDIO_FOLDER>(); }

void Settings::setAudioFolder(fs::path audioFolder) { set<SettingsElement::AUDIO_FOLDER>(audioFolder); }

auto Settings::getSizeUnit() const -> string const& { return get<SettingsElement::SIZE_UNIT>(); }

void Settings::setSizeUnit(const string& sizeUnit) { set<SettingsElement::SIZE_UNIT>(sizeUnit); }

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

void Settings::setShowPairedPages(bool showPairedPages) { set<SettingsElement::SHOW_PAIRED_PAGES>(showPairedPages); }

auto Settings::isShowPairedPages() const -> bool { return get<SettingsElement::SHOW_PAIRED_PAGES>(); }

void Settings::setPresentationMode(bool presentationMode) {
    if (presentationMode) {
        set<SettingsElement::ACTIVE_VIEW_MODE>(PresetViewModeIds::VIEW_MODE_PRESENTATION);
    }
}

auto Settings::isPresentationMode() const -> bool {
    return get<SettingsElement::ACTIVE_VIEW_MODE>() == PresetViewModeIds::VIEW_MODE_PRESENTATION;
}

void Settings::setPressureSensitivity(gboolean presureSensitivity) {
    set<SettingsElement::PRESSURE_SENSITIVITY>(presureSensitivity);
}

void Settings::setPairsOffset(int numOffset) { set<SettingsElement::NUM_PAIRS_OFFSET>(numOffset); }

auto Settings::getPairsOffset() const -> int { return get<SettingsElement::NUM_PAIRS_OFFSET>(); }

void Settings::setEmptyLastPageAppend(EmptyLastPageAppendType emptyLastPageAppend) {
    set<SettingsElement::EMPTY_LAST_PAGE_APPEND>(emptyLastPageAppend);
}

auto Settings::getEmptyLastPageAppend() const -> EmptyLastPageAppendType {
    return get<SettingsElement::EMPTY_LAST_PAGE_APPEND>();
}

void Settings::setViewColumns(int numColumns) { set<SettingsElement::NUM_COLUMNS>(numColumns); }

auto Settings::getViewColumns() const -> int { return get<SettingsElement::NUM_COLUMNS>(); }


void Settings::setViewRows(int numRows) { set<SettingsElement::NUM_ROWS>(numRows); }

auto Settings::getViewRows() const -> int { return get<SettingsElement::NUM_ROWS>(); }

void Settings::setViewFixedRows(bool viewFixedRows) { set<SettingsElement::VIEW_FIXED_ROWS>(viewFixedRows); }

auto Settings::isViewFixedRows() const -> bool { return get<SettingsElement::VIEW_FIXED_ROWS>(); }

void Settings::setViewLayoutVert(bool vert) { set<SettingsElement::LAYOUT_VERTICAL>(vert); }

auto Settings::getViewLayoutVert() const -> bool { return get<SettingsElement::LAYOUT_VERTICAL>(); }

void Settings::setViewLayoutR2L(bool r2l) { set<SettingsElement::LAYOUT_RIGHT_TO_LEFT>(r2l); }

auto Settings::getViewLayoutR2L() const -> bool { return get<SettingsElement::LAYOUT_RIGHT_TO_LEFT>(); }

void Settings::setViewLayoutB2T(bool b2t) { set<SettingsElement::LAYOUT_BOTTOM_TO_TOP>(b2t); }

auto Settings::getViewLayoutB2T() const -> bool { return get<SettingsElement::LAYOUT_BOTTOM_TO_TOP>(); }

void Settings::setLastSavePath(fs::path p) { set<SettingsElement::LAST_SAVE_PATH>(p); }

auto Settings::getLastSavePath() const -> fs::path const& { return get<SettingsElement::LAST_SAVE_PATH>(); }

void Settings::setLastOpenPath(fs::path p) { set<SettingsElement::LAST_OPEN_PATH>(p); }

auto Settings::getLastOpenPath() const -> fs::path const& { return get<SettingsElement::LAST_OPEN_PATH>(); }

void Settings::setLastImagePath(const fs::path& path) { set<SettingsElement::LAST_IMAGE_PATH>(path); }

auto Settings::getLastImagePath() const -> fs::path const& { return get<SettingsElement::LAST_IMAGE_PATH>(); }

void Settings::setZoomStep(double zoomStep) { set<SettingsElement::ZOOM_STEP>(zoomStep); }

auto Settings::getZoomStep() const -> double { return get<SettingsElement::ZOOM_STEP>(); }

void Settings::setZoomStepScroll(double zoomStepScroll) { set<SettingsElement::ZOOM_STEP_SCROLL>(zoomStepScroll); }

auto Settings::getZoomStepScroll() const -> double { return get<SettingsElement::ZOOM_STEP_SCROLL>(); }

void Settings::setEdgePanSpeed(double speed) { set<SettingsElement::EDGE_PAN_SPEED>(speed); }

auto Settings::getEdgePanSpeed() const -> double { return get<SettingsElement::EDGE_PAN_SPEED>(); }

void Settings::setEdgePanMaxMult(double maxMult) { set<SettingsElement::EDGE_PAN_MAX_MULT>(maxMult); }

auto Settings::getEdgePanMaxMult() const -> double { return get<SettingsElement::EDGE_PAN_MAX_MULT>(); }

void Settings::setDisplayDpi(int dpi) { set<SettingsElement::DISPLAY_DPI>(dpi); }

auto Settings::getDisplayDpi() const -> int { return get<SettingsElement::DISPLAY_DPI>(); }

void Settings::setDarkTheme(bool dark) { set<SettingsElement::DARK_THEME>(dark); }

auto Settings::isDarkTheme() const -> bool { return get<SettingsElement::DARK_THEME>(); }

void Settings::setAreStockIconsUsed(bool use) { set<SettingsElement::USE_STOCK_ICONS>(use); }

auto Settings::areStockIconsUsed() const -> bool { return get<SettingsElement::USE_STOCK_ICONS>(); }

auto Settings::isFullscreen() const -> bool { return getViewModes()[getActiveViewMode()].goFullscreen; }

auto Settings::isSidebarVisible() const -> bool { return get<SettingsElement::SHOW_SIDEBAR>(); }

void Settings::setSidebarVisible(bool visible) { set<SettingsElement::SHOW_SIDEBAR>(visible); }

auto Settings::isToolbarVisible() const -> bool { return get<SettingsElement::SHOW_TOOLBAR>(); }

void Settings::setToolbarVisible(bool visible) { set<SettingsElement::SHOW_TOOLBAR>(visible); }

auto Settings::getSidebarWidth() const -> int { return get<SettingsElement::SIDEBAR_WIDTH>(); }

void Settings::setSidebarWidth(int width) { set<SettingsElement::SIDEBAR_WIDTH>(width); }

void Settings::setMainWndSize(int width, int height) {
    set<SettingsElement::MAIN_WINDOW_WIDTH>(width);
    set<SettingsElement::MAIN_WINDOW_HEIGHT>(height);
}

auto Settings::getMainWndWidth() const -> int { return get<SettingsElement::MAIN_WINDOW_WIDTH>(); }

auto Settings::getMainWndHeight() const -> int { return get<SettingsElement::MAIN_WINDOW_HEIGHT>(); }

auto Settings::isMainWndMaximized() const -> bool { return get<SettingsElement::MAXIMIZED>(); }

void Settings::setMainWndMaximized(bool max) { set<SettingsElement::MAXIMIZED>(max); }

void Settings::setSelectedToolbar(const string& name) { set<SettingsElement::SELECTED_TOOLBAR>(name); }

auto Settings::getSelectedToolbar() const -> string const& { return get<SettingsElement::SELECTED_TOOLBAR>(); }

auto Settings::getButtonConfig(unsigned int id) -> ButtonConfig* {
    auto cfg = get<SettingsElement::NESTED_BUTTON_CONFIG>();
    if (id >= cfg.size()) {
        g_error("Settings::getButtonConfig try to get id=%i out of range!", id);
        return nullptr;
    }
    return cfg[id].get();
}

void Settings::setViewMode(ViewModeId mode, ViewMode viewMode) {
    switch (mode) {
        case PresetViewModeIds::VIEW_MODE_DEFAULT:
            set<SettingsElement::DEFAULT_VIEW_MODE_ATTRIBUTES>(viewMode);
            break;
        case PresetViewModeIds::VIEW_MODE_FULLSCREEN:
            set<SettingsElement::FULLSCREEN_VIEW_MODE_ATTRIBUTES>(viewMode);
            break;
        case PresetViewModeIds::VIEW_MODE_PRESENTATION:
            set<SettingsElement::PRESENTATION_VIEW_MODE_ATTRIBUTES>(viewMode);
            break;
        default:
            g_warning("Settings::Unknown ViewModeId '%d' in setViewMode", static_cast<int>(mode));
    }
}

auto Settings::getTouchZoomStartThreshold() const -> double {
    return get<SettingsElement::TOUCH_ZOOM_START_THRESHOLD>();
}
void Settings::setTouchZoomStartThreshold(double threshold) {
    set<SettingsElement::TOUCH_ZOOM_START_THRESHOLD>(threshold);
}


auto Settings::getPDFPageRerenderThreshold() const -> double { return get<SettingsElement::PAGE_RERENDER_THRESHOLD>(); }
void Settings::setPDFPageRerenderThreshold(double threshold) {
    set<SettingsElement::PAGE_RERENDER_THRESHOLD>(threshold);
}

auto Settings::getPdfPageCacheSize() const -> int { return get<SettingsElement::PDF_PAGE_CACHE_SIZE>(); }

void Settings::setPdfPageCacheSize(int size) { set<SettingsElement::PDF_PAGE_CACHE_SIZE>(size); }

auto Settings::getPreloadPagesBefore() const -> unsigned int { return get<SettingsElement::PRELOAD_PAGES_BEFORE>(); }

void Settings::setPreloadPagesBefore(unsigned int n) { set<SettingsElement::PRELOAD_PAGES_BEFORE>(n); }

auto Settings::getPreloadPagesAfter() const -> unsigned int { return get<SettingsElement::PRELOAD_PAGES_AFTER>(); }

void Settings::setPreloadPagesAfter(unsigned int n) { set<SettingsElement::PRELOAD_PAGES_AFTER>(n); }

auto Settings::isEagerPageCleanup() const -> bool { return get<SettingsElement::EAGER_PAGE_CLEANUP>(); }

void Settings::setEagerPageCleanup(bool b) { set<SettingsElement::EAGER_PAGE_CLEANUP>(b); }

auto Settings::getBorderColor() const -> Color { return get<SettingsElement::SELECTION_BORDER_COLOR>(); }

void Settings::setBorderColor(Color color) { set<SettingsElement::SELECTION_BORDER_COLOR>(color); }

auto Settings::getSelectionColor() const -> Color { return get<SettingsElement::SELECTION_MARKER_COLOR>(); }

void Settings::setSelectionColor(Color color) { set<SettingsElement::SELECTION_MARKER_COLOR>(color); }

auto Settings::getActiveSelectionColor() const -> Color { return get<SettingsElement::ACTIVE_SELECTION_COLOR>(); }

void Settings::setActiveSelectionColor(Color color) { set<SettingsElement::ACTIVE_SELECTION_COLOR>(color); }

auto Settings::getBackgroundColor() const -> Color { return get<SettingsElement::BACKGROUND_COLOR>(); }

void Settings::setBackgroundColor(Color color) { set<SettingsElement::BACKGROUND_COLOR>(color); }

auto Settings::getFont() -> XojFont { return get<SettingsElement::FONT>(); }

void Settings::setFont(const XojFont& font) { set<SettingsElement::FONT>(font); }


auto Settings::getAudioInputDevice() const -> PaDeviceIndex { return get<SettingsElement::AUDIO_INPUT_DEVICE>(); }

void Settings::setAudioInputDevice(PaDeviceIndex deviceIndex) { set<SettingsElement::AUDIO_INPUT_DEVICE>(deviceIndex); }

auto Settings::getAudioOutputDevice() const -> PaDeviceIndex { return get<SettingsElement::AUDIO_OUTPUT_DEVICE>(); }

void Settings::setAudioOutputDevice(PaDeviceIndex deviceIndex) {
    set<SettingsElement::AUDIO_OUTPUT_DEVICE>(deviceIndex);
}

auto Settings::getAudioSampleRate() const -> double { return get<SettingsElement::AUDIO_SAMPLE_RATE>(); }

void Settings::setAudioSampleRate(double sampleRate) { set<SettingsElement::AUDIO_SAMPLE_RATE>(sampleRate); }

auto Settings::getAudioGain() const -> double { return get<SettingsElement::AUDIO_GAIN>(); }

void Settings::setAudioGain(double gain) { set<SettingsElement::AUDIO_GAIN>(gain); }

auto Settings::getDefaultSeekTime() const -> unsigned int { return get<SettingsElement::DEFAULT_SEEK_TIME>(); }

void Settings::setDefaultSeekTime(unsigned int t) { set<SettingsElement::DEFAULT_SEEK_TIME>(t); }

auto Settings::getPluginEnabled() const -> string const& { return get<SettingsElement::PLUGIN_ENABLED>(); }

void Settings::setPluginEnabled(const string& pluginEnabled) { set<SettingsElement::PLUGIN_ENABLED>(pluginEnabled); }

auto Settings::getPluginDisabled() const -> string const& { return get<SettingsElement::PLUGIN_DISABLED>(); }

void Settings::setPluginDisabled(const string& pluginDisabled) {
    set<SettingsElement::PLUGIN_DISABLED>(pluginDisabled);
}


void Settings::getStrokeFilter(int* ignoreTime, double* ignoreLength, int* successiveTime) const {
    *ignoreTime = get<SettingsElement::STROKE_FILTER_IGNORE_TIME>();
    *ignoreLength = get<SettingsElement::STROKE_FILTER_IGNORE_LENGTH>();
    *successiveTime = get<SettingsElement::STROKE_FILTER_SUCCESSIVE_TIME>();
}

void Settings::setStrokeFilter(int ignoreTime, double ignoreLength, int successiveTime) {
    set<SettingsElement::STROKE_FILTER_IGNORE_TIME>(ignoreTime);
    set<SettingsElement::STROKE_FILTER_IGNORE_LENGTH>(ignoreLength);
    set<SettingsElement::STROKE_FILTER_SUCCESSIVE_TIME>(successiveTime);
}

void Settings::setStrokeFilterEnabled(bool enabled) { set<SettingsElement::STROKE_FILTER_ENABLED>(enabled); }

auto Settings::getStrokeFilterEnabled() const -> bool { return get<SettingsElement::STROKE_FILTER_ENABLED>(); }

void Settings::setDoActionOnStrokeFiltered(bool enabled) {
    set<SettingsElement::DO_ACTION_ON_STROKE_FILTERED>(enabled);
}

auto Settings::getDoActionOnStrokeFiltered() const -> bool {
    return get<SettingsElement::DO_ACTION_ON_STROKE_FILTERED>();
}

void Settings::setTrySelectOnStrokeFiltered(bool enabled) {
    set<SettingsElement::TRY_SELECT_ON_STROKE_FILTERED>(enabled);
}

auto Settings::getTrySelectOnStrokeFiltered() const -> bool {
    return get<SettingsElement::TRY_SELECT_ON_STROKE_FILTERED>();
}

void Settings::setSnapRecognizedShapesEnabled(bool enabled) { set<SettingsElement::SNAP_RECOGNIZED_SHAPES>(enabled); }

auto Settings::getSnapRecognizedShapesEnabled() const -> bool { return get<SettingsElement::SNAP_RECOGNIZED_SHAPES>(); }


void Settings::setRestoreLineWidthEnabled(bool enabled) { set<SettingsElement::RESTORE_LINE_WIDTH>(enabled); }

auto Settings::getRestoreLineWidthEnabled() const -> bool { return get<SettingsElement::RESTORE_LINE_WIDTH>(); }

auto Settings::setPreferredLocale(std::string const& locale) -> void { set<SettingsElement::PREFERRED_LOCALE>(locale); }

auto Settings::getPreferredLocale() const -> std::string { return get<SettingsElement::PREFERRED_LOCALE>(); }

void Settings::setIgnoredStylusEvents(int numEvents) { set<SettingsElement::NUM_IGNORED_STYLUS_EVENTS>(numEvents); }

auto Settings::getIgnoredStylusEvents() const -> int { return get<SettingsElement::NUM_IGNORED_STYLUS_EVENTS>(); }

void Settings::setInputSystemTPCButtonEnabled(bool tpcButtonEnabled) {
    set<SettingsElement::INPUT_SYSTEM_TPC_BUTTON>(tpcButtonEnabled);
}

auto Settings::getInputSystemTPCButtonEnabled() const -> bool {
    return get<SettingsElement::INPUT_SYSTEM_TPC_BUTTON>();
}

void Settings::setInputSystemDrawOutsideWindowEnabled(bool drawOutsideWindowEnabled) {
    set<SettingsElement::INPUT_SYSTEM_DRAW_OUTSIDE_WINDOW>(drawOutsideWindowEnabled);
}

auto Settings::getInputSystemDrawOutsideWindowEnabled() const -> bool {
    return get<SettingsElement::INPUT_SYSTEM_DRAW_OUTSIDE_WINDOW>();
}

void Settings::setDeviceClassForDevice(GdkDevice* device, InputDeviceTypeOption deviceClass) {
    this->setDeviceClassForDevice(gdk_device_get_name(device), gdk_device_get_source(device), deviceClass);
}

void Settings::setDeviceClassForDevice(const string& deviceName, GdkInputSource deviceSource,
                                       InputDeviceTypeOption deviceClass) {
    std::map<std::string, std::pair<InputDeviceTypeOption, GdkInputSource>> devClass =
            get<SettingsElement::NESTED_DEVICE_CLASSES>();
    auto it = devClass.find(deviceName);
    if (it != devClass.end()) {
        it->second.first = deviceClass;
        it->second.second = deviceSource;
    } else {
        devClass.emplace(deviceName, std::make_pair(deviceClass, deviceSource));
    }
    set<SettingsElement::NESTED_DEVICE_CLASSES>(devClass);
}

auto Settings::getKnownInputDevices() const -> std::vector<InputDevice> {
    auto devClass = get<SettingsElement::NESTED_DEVICE_CLASSES>();
    std::vector<InputDevice> inputDevices;
    for (auto pair: devClass) {
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
    auto devClass = get<SettingsElement::NESTED_DEVICE_CLASSES>();
    auto search = devClass.find(deviceName);
    if (search != devClass.end()) {
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

auto Settings::isScrollbarFadeoutDisabled() const -> bool { return get<SettingsElement::DISABLE_SCROLLBAR_FADEOUT>(); }

void Settings::setScrollbarFadeoutDisabled(bool disable) { set<SettingsElement::DISABLE_SCROLLBAR_FADEOUT>(disable); }

auto Settings::isAudioDisabled() const -> bool { return get<SettingsElement::DISABLE_AUDIO>(); }

void Settings::setAudioDisabled(bool disable) { set<SettingsElement::DISABLE_AUDIO>(disable); }

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

auto SElement::attributes() const -> std::map<string, SAttribute>& { return this->element->attributes; }

auto SElement::children() const -> std::map<string, SElement>& { return this->element->children; }

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

auto SElement::getDouble(const string& name, double& value) const -> bool {
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

auto SElement::getInt(const string& name, int& value) const -> bool {
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

auto SElement::getBool(const string& name, bool& value) const -> bool {
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

auto SElement::getString(const string& name, string& value) const -> bool {
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
auto Settings::getStabilizerCuspDetection() const -> bool { return get<SettingsElement::STABILIZER_CUSP_DETECTION>(); }
auto Settings::getStabilizerFinalizeStroke() const -> bool {
    return get<SettingsElement::STABILIZER_FINALIZE_STROKE>();
}
auto Settings::getStabilizerBuffersize() const -> size_t { return get<SettingsElement::STABILIZER_BUFFERSIZE>(); }
auto Settings::getStabilizerDeadzoneRadius() const -> double {
    return get<SettingsElement::STABILIZER_DEADZONE_RADIUS>();
}
auto Settings::getStabilizerDrag() const -> double { return get<SettingsElement::STABILIZER_DRAG>(); }
auto Settings::getStabilizerMass() const -> double { return get<SettingsElement::STABILIZER_MASS>(); }
auto Settings::getStabilizerSigma() const -> double { return get<SettingsElement::STABILIZER_SIGMA>(); }
auto Settings::getStabilizerAveragingMethod() const -> StrokeStabilizer::AveragingMethod {
    return get<SettingsElement::STABILIZER_AVERAGING_METHOD>();
}
auto Settings::getStabilizerPreprocessor() const -> StrokeStabilizer::Preprocessor {
    return get<SettingsElement::STABILIZER_PREPROCESSOR>();
}

void Settings::setStabilizerCuspDetection(bool cuspDetection) {
    set<SettingsElement::STABILIZER_CUSP_DETECTION>(cuspDetection);
}
void Settings::setStabilizerFinalizeStroke(bool finalizeStroke) {
    set<SettingsElement::STABILIZER_FINALIZE_STROKE>(finalizeStroke);
}
void Settings::setStabilizerBuffersize(size_t buffersize) { set<SettingsElement::STABILIZER_BUFFERSIZE>(buffersize); }
void Settings::setStabilizerDeadzoneRadius(double deadzoneRadius) {
    set<SettingsElement::STABILIZER_DEADZONE_RADIUS>(deadzoneRadius);
}
void Settings::setStabilizerDrag(double drag) { set<SettingsElement::STABILIZER_DRAG>(drag); }
void Settings::setStabilizerMass(double mass) { set<SettingsElement::STABILIZER_MASS>(mass); }
void Settings::setStabilizerSigma(double sigma) { set<SettingsElement::STABILIZER_SIGMA>(sigma); }
void Settings::setStabilizerAveragingMethod(StrokeStabilizer::AveragingMethod averagingMethod) {
    set<SettingsElement::STABILIZER_AVERAGING_METHOD>(averagingMethod);
}
void Settings::setStabilizerPreprocessor(StrokeStabilizer::Preprocessor preprocessor) {
    set<SettingsElement::STABILIZER_PREPROCESSOR>(preprocessor);
}

/**
 * @brief Get Color Palette used for Tools
 *
 * @return Palette&
 */
auto Settings::getColorPalette() -> const Palette& { return *(this->palette); }


void Settings::setUseSpacesAsTab(bool useSpaces) { set<SettingsElement::USE_SPACES_AS_TAB>(useSpaces); }
bool Settings::getUseSpacesAsTab() const { return get<SettingsElement::USE_SPACES_AS_TAB>(); }

void Settings::setNumberOfSpacesForTab(unsigned int numberOfSpaces) {
    set<SettingsElement::NUMBER_OF_SPACES_FOR_TAB>(numberOfSpaces);
}

unsigned int Settings::getNumberOfSpacesForTab() const { return get<SettingsElement::NUMBER_OF_SPACES_FOR_TAB>(); }
