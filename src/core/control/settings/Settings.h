/*
 * Xournal++
 *
 * Xournal Settings
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <array>    // for array
#include <cstddef>  // for size_t
#include <map>      // for map
#include <memory>   // for make_shared, shared_ptr
#include <string>   // for string, basic_string
#include <utility>  // for pair
#include <vector>   // for vector

#include <gdk/gdk.h>                      // for GdkInputSource, GdkD...
#include <glib.h>                         // for gchar, gboolean, gint
#include <libxml/tree.h>                  // for xmlNodePtr, xmlDocPtr
#include <portaudiocpp/PortAudioCpp.hxx>  // for PaDeviceIndex

#include "control/tools/StrokeStabilizerEnum.h"  // for AveragingMethod, Pre...
#include "model/Font.h"                          // for XojFont
#include "util/Color.h"                          // for Color

#include "LatexSettings.h"  // for LatexSettings
#include "SettingsEnums.h"  // for InputDeviceTypeOption
#include "ViewModes.h"      // for ViewModes
#include "filesystem.h"     // for path

struct Palette;

constexpr auto DEFAULT_GRID_SIZE = 14.17;

class ButtonConfig;
class InputDevice;

class SAttribute {
public:
    SAttribute();
    SAttribute(const SAttribute& attrib);
    virtual ~SAttribute();

public:
    std::string sValue;
    int iValue{};
    double dValue{};

    AttributeType type;

    std::string comment;
};


class SElement final {
    struct SElementData {
    private:
        std::map<std::string, SAttribute> attributes;
        std::map<std::string, SElement> children;
        friend class SElement;
    };

public:
    SElement() = default;

    void clear();

    SElement& child(const std::string& name);

    void setIntHex(const std::string& name, const int value);
    void setInt(const std::string& name, const int value);
    void setDouble(const std::string& name, const double value);
    void setBool(const std::string& name, const bool value);
    void setString(const std::string& name, const std::string& value);

    [[maybe_unused]] void setComment(const std::string& name, const std::string& comment);

    bool getInt(const std::string& name, int& value);
    [[maybe_unused]] bool getDouble(const std::string& name, double& value);
    bool getBool(const std::string& name, bool& value);
    bool getString(const std::string& name, std::string& value);

    std::map<std::string, SAttribute>& attributes();
    std::map<std::string, SElement>& children();

private:
    std::shared_ptr<SElementData> element = std::make_shared<SElementData>();
};

class Settings {
public:
    /*[[implicit]]*/ Settings(fs::path filepath);
    Settings(const Settings& settings) = delete;
    void operator=(const Settings& settings) = delete;
    virtual ~Settings();

public:
    bool load();
    void parseData(xmlNodePtr cur, SElement& elem);

    void save();

private:
    void loadDefault();
    void parseItem(xmlDocPtr doc, xmlNodePtr cur);

    static xmlNodePtr savePropertyDouble(const gchar* key, double value, xmlNodePtr parent);
    static xmlNodePtr saveProperty(const gchar* key, int value, xmlNodePtr parent);
    static xmlNodePtr savePropertyUnsigned(const gchar* key, unsigned int value, xmlNodePtr parent);
    static xmlNodePtr saveProperty(const gchar* key, const gchar* value, xmlNodePtr parent);

    void saveData(xmlNodePtr root, const std::string& name, SElement& elem);

    void saveButtonConfig();
    void loadButtonConfig();

public:
    // View Mode
    bool loadViewMode(ViewModeId mode);

    // Getter- / Setter
    const std::vector<ViewMode>& getViewModes() const;

    bool isPressureSensitivity() const;
    void setPressureSensitivity(gboolean presureSensitivity);

    /**
     * Input device pressure options
     */
    double getMinimumPressure() const;
    void setMinimumPressure(double minimumPressure);

    double getPressureMultiplier() const;
    void setPressureMultiplier(double multiplier);

    /**
     * Getter, enable/disable
     */
    bool isZoomGesturesEnabled() const;
    void setZoomGesturesEnabled(bool enable);

    /**
     * The last used font
     */
    XojFont& getFont();
    void setFont(const XojFont& font);

    /**
     * The selected Toolbar
     */
    void setSelectedToolbar(const std::string& name);
    std::string const& getSelectedToolbar() const;

    void setEdgePanSpeed(double speed);
    double getEdgePanSpeed() const;

    void setEdgePanMaxMult(double mult);
    double getEdgePanMaxMult() const;

    /**
     * Set the Zoomstep for one step in percent
     */
    void setZoomStep(double zoomStep);
    double getZoomStep() const;

    /**
     * Set the Zoomstep for Ctrl + Scroll in percent
     */
    void setZoomStepScroll(double zoomStepScroll);
    double getZoomStepScroll() const;

    /**
     * Sets the screen resolution in DPI
     */
    void setDisplayDpi(int dpi);
    int getDisplayDpi() const;

    /**
     * Dark theme for white-coloured icons
     */
    void setDarkTheme(bool dark);
    bool isDarkTheme() const;

    void setAreStockIconsUsed(bool use);
    bool areStockIconsUsed() const;

    /**
     * The last saved path
     */
    void setLastSavePath(fs::path p);
    fs::path const& getLastSavePath() const;

    /**
     * The last open path
     */
    void setLastOpenPath(fs::path p);
    fs::path const& getLastOpenPath() const;

    void setLastImagePath(const fs::path& p);
    fs::path const& getLastImagePath() const;

    void setMainWndSize(int width, int height);
    void setMainWndMaximized(bool max);
    int getMainWndWidth() const;
    int getMainWndHeight() const;
    bool isMainWndMaximized() const;

    bool isFullscreen() const;

    bool isSidebarVisible() const;
    void setSidebarVisible(bool visible);

    bool isToolbarVisible() const;
    void setToolbarVisible(bool visible);

    int getSidebarWidth() const;
    void setSidebarWidth(int width);

    bool isSidebarOnRight() const;
    void setSidebarOnRight(bool right);

    bool isScrollbarOnLeft() const;
    void setScrollbarOnLeft(bool right);

    bool isMenubarVisible() const;
    void setMenubarVisible(bool visible);

    const bool isFilepathInTitlebarShown() const;
    void setFilepathInTitlebarShown(const bool shown);

    void setShowPairedPages(bool showPairedPages);
    bool isShowPairedPages() const;

    void setPresentationMode(bool presentationMode);
    bool isPresentationMode() const;

    void setPairsOffset(int numOffset);
    int getPairsOffset() const;

    void setEmptyLastPageAppend(EmptyLastPageAppendType emptyLastPageAppend);
    EmptyLastPageAppendType getEmptyLastPageAppend() const;

    void setViewColumns(int numColumns);
    int getViewColumns() const;

    void setViewRows(int numRows);
    int getViewRows() const;

    void setViewFixedRows(bool viewFixedRows);
    bool isViewFixedRows() const;

    void setViewLayoutVert(bool vert);
    bool getViewLayoutVert() const;

    void setViewLayoutR2L(bool r2l);
    bool getViewLayoutR2L() const;

    void setViewLayoutB2T(bool b2t);
    bool getViewLayoutB2T() const;

    bool isAutoloadMostRecent() const;
    void setAutoloadMostRecent(bool load);

    bool isAutoloadPdfXoj() const;
    void setAutoloadPdfXoj(bool load);

    int getAutosaveTimeout() const;
    void setAutosaveTimeout(int autosave);
    bool isAutosaveEnabled() const;
    void setAutosaveEnabled(bool autosave);

    bool getAddVerticalSpace() const;
    void setAddVerticalSpace(bool space);
    int getAddVerticalSpaceAmount() const;
    void setAddVerticalSpaceAmount(int pixels);

    bool getAddHorizontalSpace() const;
    void setAddHorizontalSpace(bool space);
    int getAddHorizontalSpaceAmount() const;
    void setAddHorizontalSpaceAmount(int pixels);

    bool getDrawDirModsEnabled() const;
    void setDrawDirModsEnabled(bool enable);
    int getDrawDirModsRadius() const;
    void setDrawDirModsRadius(int pixels);

    bool getTouchDrawingEnabled() const;
    void setTouchDrawingEnabled(bool b);

    bool getGtkTouchInertialScrollingEnabled() const;
    void setGtkTouchInertialScrollingEnabled(bool b);

    bool isPressureGuessingEnabled() const;
    void setPressureGuessingEnabled(bool b);

    bool isSnapRotation() const;
    void setSnapRotation(bool b);
    double getSnapRotationTolerance() const;
    void setSnapRotationTolerance(double tolerance);

    bool isSnapGrid() const;
    void setSnapGrid(bool b);
    double getSnapGridTolerance() const;
    void setSnapGridTolerance(double tolerance);
    double getSnapGridSize() const;
    void setSnapGridSize(double gridSize);

    double getStrokeRecognizerMinSize() const;
    void setStrokeRecognizerMinSize(double value);

    StylusCursorType getStylusCursorType() const;
    void setStylusCursorType(StylusCursorType stylusCursorType);

    EraserVisibility getEraserVisibility() const;
    void setEraserVisibility(EraserVisibility eraserVisibility);

    IconTheme getIconTheme() const;
    void setIconTheme(IconTheme iconTheme);

    SidebarNumberingStyle getSidebarNumberingStyle() const;
    void setSidebarNumberingStyle(SidebarNumberingStyle numberingStyle);

    bool isHighlightPosition() const;
    void setHighlightPosition(bool highlight);

    Color getCursorHighlightColor() const;
    void setCursorHighlightColor(Color color);

    double getCursorHighlightRadius() const;
    void setCursorHighlightRadius(double radius);

    Color getCursorHighlightBorderColor() const;
    void setCursorHighlightBorderColor(Color color);

    double getCursorHighlightBorderWidth() const;
    void setCursorHighlightBorderWidth(double width);

    ScrollbarHideType getScrollbarHideType() const;
    void setScrollbarHideType(ScrollbarHideType type);

    bool isScrollbarFadeoutDisabled() const;
    void setScrollbarFadeoutDisabled(bool disable);

    std::string const& getDefaultSaveName() const;
    void setDefaultSaveName(const std::string& name);

    std::string const& getDefaultPdfExportName() const;
    void setDefaultPdfExportName(const std::string& name);

    ButtonConfig* getButtonConfig(unsigned int id);

    void setViewMode(ViewModeId mode, ViewMode ViewMode);

    Color getBorderColor() const;
    void setBorderColor(Color color);

    Color getSelectionColor() const;
    void setSelectionColor(Color color);

    Color getBackgroundColor() const;
    void setBackgroundColor(Color color);

    // Re-render pages if document zoom differs from the last render zoom by the given threshold.
    double getPDFPageRerenderThreshold() const;
    void setPDFPageRerenderThreshold(double threshold);

    double getTouchZoomStartThreshold() const;
    void setTouchZoomStartThreshold(double threshold);

    int getPdfPageCacheSize() const;
    [[maybe_unused]] void setPdfPageCacheSize(int size);

    unsigned int getPreloadPagesBefore() const;
    void setPreloadPagesBefore(unsigned int n);

    unsigned int getPreloadPagesAfter() const;
    void setPreloadPagesAfter(unsigned int n);

    bool isEagerPageCleanup() const;
    void setEagerPageCleanup(bool b);

    std::string const& getPageTemplate() const;
    void setPageTemplate(const std::string& pageTemplate);

    fs::path const& getAudioFolder() const;
    void setAudioFolder(fs::path audioFolder);

    static constexpr PaDeviceIndex AUDIO_INPUT_SYSTEM_DEFAULT = -1;
    PaDeviceIndex getAudioInputDevice() const;
    void setAudioInputDevice(PaDeviceIndex deviceIndex);

    static constexpr PaDeviceIndex AUDIO_OUTPUT_SYSTEM_DEFAULT = -1;
    PaDeviceIndex getAudioOutputDevice() const;
    void setAudioOutputDevice(PaDeviceIndex deviceIndex);

    double getAudioSampleRate() const;
    void setAudioSampleRate(double sampleRate);

    double getAudioGain() const;
    void setAudioGain(double gain);

    unsigned int getDefaultSeekTime() const;
    void setDefaultSeekTime(unsigned int t);

    std::string const& getPluginEnabled() const;
    void setPluginEnabled(const std::string& pluginEnabled);

    std::string const& getPluginDisabled() const;
    void setPluginDisabled(const std::string& pluginDisabled);

    /**
     * Sets #numIgnoredStylusEvents. If given a negative value writes 0 instead.
     */
    void setIgnoredStylusEvents(int numEvents);
    /**
     * Returns #numIgnoredStylusEvents.
     */
    int getIgnoredStylusEvents() const;

    bool getInputSystemTPCButtonEnabled() const;
    void setInputSystemTPCButtonEnabled(bool tpcButtonEnabled);

    bool getInputSystemDrawOutsideWindowEnabled() const;
    void setInputSystemDrawOutsideWindowEnabled(bool drawOutsideWindowEnabled);

    void loadDeviceClasses();
    void saveDeviceClasses();
    void setDeviceClassForDevice(GdkDevice* device, InputDeviceTypeOption deviceClass);
    void setDeviceClassForDevice(const std::string& deviceName, GdkInputSource deviceSource,
                                 InputDeviceTypeOption deviceClass);
    InputDeviceTypeOption getDeviceClassForDevice(GdkDevice* device) const;
    InputDeviceTypeOption getDeviceClassForDevice(const std::string& deviceName, GdkInputSource deviceSource) const;
    std::vector<InputDevice> getKnownInputDevices() const;

    /**
     * Get name, e.g. "cm"
     */
    std::string const& getSizeUnit() const;

    /**
     * Get size index in XOJ_UNITS
     */
    int getSizeUnitIndex() const;

    /**
     * Set Unit, e.g. "cm"
     */
    void setSizeUnit(const std::string& sizeUnit);

    /**
     * Set size index in XOJ_UNITS
     */
    void setSizeUnitIndex(int sizeUnitId);

    /**
     * Set StrokeFilter enabled
     */
    void setStrokeFilterEnabled(bool enabled);

    /**
     * Get StrokeFilter enabled
     */
    bool getStrokeFilterEnabled() const;

    /**
     * get strokeFilter settings
     */
    void getStrokeFilter(int* strokeFilterIgnoreTime, double* strokeFilterIgnoreLength,
                         int* strokeFilterSuccessiveTime) const;

    /**
     * configure stroke filter
     */
    void setStrokeFilter(int strokeFilterIgnoreTime, double strokeFilterIgnoreLength, int strokeFilterSuccessiveTime);

    /**
     * Set DoActionOnStrokeFilter enabled
     */
    void setDoActionOnStrokeFiltered(bool enabled);

    /**
     * Get DoActionOnStrokeFilter enabled
     */
    bool getDoActionOnStrokeFiltered() const;

    /**
     * Set TrySelectOnStrokeFilter enabled
     */
    void setTrySelectOnStrokeFiltered(bool enabled);

    /**
     * Get TrySelectOnStrokeFilter enabled
     */
    bool getTrySelectOnStrokeFiltered() const;

    /**
     * Set snap recognized shapes enabled
     */
    void setSnapRecognizedShapesEnabled(bool enabled);

    /**
     * Get snap recognized shapes enabled
     */
    bool getSnapRecognizedShapesEnabled() const;

    /**
     * Set line width restoring for resized edit selctions enabled
     */
    void setRestoreLineWidthEnabled(bool enabled);

    /**
     * Get line width restoring enabled
     */
    bool getRestoreLineWidthEnabled() const;

    /**
     * Set the preferred locale
     */
    void setPreferredLocale(std::string const& locale);

    /**
     * Get the preferred locale
     */
    std::string getPreferredLocale() const;

    /**
     * Stabilizer related getters and setters
     */
    bool getStabilizerCuspDetection() const;
    bool getStabilizerFinalizeStroke() const;
    size_t getStabilizerBuffersize() const;
    double getStabilizerDeadzoneRadius() const;
    double getStabilizerDrag() const;
    double getStabilizerMass() const;
    double getStabilizerSigma() const;
    StrokeStabilizer::AveragingMethod getStabilizerAveragingMethod() const;
    StrokeStabilizer::Preprocessor getStabilizerPreprocessor() const;

    void setStabilizerCuspDetection(bool cuspDetection);
    void setStabilizerFinalizeStroke(bool finalizeStroke);
    void setStabilizerBuffersize(size_t buffersize);
    void setStabilizerDeadzoneRadius(double deadzoneRadius);
    void setStabilizerDrag(double drag);
    void setStabilizerMass(double mass);
    void setStabilizerSigma(double sigma);
    void setStabilizerAveragingMethod(StrokeStabilizer::AveragingMethod averagingMethod);
    void setStabilizerPreprocessor(StrokeStabilizer::Preprocessor preprocessor);

    const Palette& getColorPalette();

public:
    // Custom settings
    SElement& getCustomElement(const std::string& name);

    /**
     * Call this after you have done all custom settings changes
     */
    void customSettingsChanged();

    /**
     * Do not save settings until transactionEnd() is called
     */
    void transactionStart();

    /**
     * Stop transaction and save settings
     */
    void transactionEnd();

    LatexSettings latexSettings{};

private:
    /**
     *  The config filepath
     */
    fs::path filepath;

private:
    /**
     * The settings tree
     */
    std::map<std::string, SElement> data;

    /**
     *  Use pen pressure to control stroke width?
     */
    bool pressureSensitivity{};

    /**
     * Adjust input pressure?
     */
    double minimumPressure{};
    double pressureMultiplier{};

    /**
     * If the touch zoom gestures are enabled
     */
    bool zoomGesturesEnabled{};

    /**
     *  If fullscreen is active
     */
    bool fullscreenActive{};

    /**
     *  If the sidebar is visible
     */
    bool showSidebar{};

    /**
     *  If the sidebar is visible
     */
    bool showToolbar{};

    /**
     *  The Width of the Sidebar
     */
    int sidebarWidth{};

    /**
     *  If the sidebar is on the right
     */
    bool sidebarOnRight{};

    /**
     *  Type of cursor icon to use with a stylus
     */
    StylusCursorType stylusCursorType;

    /**
     * Visibility of eraser cursor
     */
    EraserVisibility eraserVisibility;

    /**
     * Icon Theme
     */
    IconTheme iconTheme;

    /**
     * Sidebar page number style
     */
    SidebarNumberingStyle sidebarNumberingStyle;

    /**
     * Show a colored circle around the cursor
     */
    bool highlightPosition{};

    /**
     * Cursor highlight color (ARGB format)
     */
    Color cursorHighlightColor{};

    /**
     * Radius of cursor highlight circle. Note that this is limited by the size
     * of the cursor in the display server (default is probably 30 pixels).
     */
    double cursorHighlightRadius{};

    /**
     * Cursor highlight border color (ARGB format)
     */
    Color cursorHighlightBorderColor{};

    /**
     * Width of cursor highlight border, in pixels.
     */
    double cursorHighlightBorderWidth{};

    /**
     * If the user uses a dark-themed DE, he should enable this
     * (white icons)
     */
    bool darkTheme{};

    /**
     * If stock icons are used instead of Xournal++ icons when available
     */
    bool useStockIcons{};

    /**
     * If the menu bar is visible on startup
     */
    bool menubarVisible{};

    /**
     * If the filepath is shown in titlebar
     */
    bool filepathShownInTitlebar{};

    /**
     *  Hide the scrollbar
     */
    ScrollbarHideType scrollbarHideType;

    /**
     * Disable scrollbar fade out (overlay scrolling)
     */
    bool disableScrollbarFadeout{};

    /**
     *  The selected Toolbar name
     */
    std::string selectedToolbar;

    /**
     *  The last saved folder
     */
    fs::path lastSavePath;

    /**
     *  The last opened folder
     */
    fs::path lastOpenPath;

    /**
     *  The last "insert image" folder
     */
    fs::path lastImagePath;

    /**
     * The last used font
     */
    XojFont font;

    /**
     * Base speed (as a percentage of visible canvas) of edge pan per
     * second
     */
    double edgePanSpeed{};

    /**
     * Maximum multiplier of edge pan speed due to proportion of selection out
     * of view
     */
    double edgePanMaxMult{};

    /**
     * Zoomstep for one step
     */
    double zoomStep{};

    /**
     * Zoomstep for Ctrl + Scroll zooming
     */
    double zoomStepScroll{};

    /**
     * The display resolution, in pixels per inch
     */
    gint displayDpi{};

    /**
     *  If the window is maximized
     */
    bool maximized{};

    /**
     * Width of the main window
     */
    int mainWndWidth{};

    /**
     * Height of the main window
     */
    int mainWndHeight{};

    /**
     * Show the scrollbar on the left side
     */
    bool scrollbarOnLeft{};

    /**
     *  Pairs pages
     */
    bool showPairedPages{};

    /**
     *  Sets presentation mode
     */
    bool presentationMode{};

    /**
     *  Offsets first page ( to align pairing )
     */
    int numPairsOffset{};

    /**
     * Preference for appending an empty last page to the document
     */
    EmptyLastPageAppendType emptyLastPageAppend{};

    /**
     *  Use when fixed number of columns
     */
    int numColumns{};

    /**
     *  Use when fixed number of rows
     */
    int numRows{};

    /**
     *  USE  fixed rows, otherwise fixed columns
     */
    bool viewFixedRows{};

    /**
     *  Layout Vertical then Horizontal
     */
    bool layoutVertical{};

    /**
     *  Layout pages right to left
     */
    bool layoutRightToLeft{};

    /**
     *  Layout Bottom to Top
     */
    bool layoutBottomToTop{};


    /**
     * Automatically load filename.pdf.xoj / .pdf.xopp instead of filename.pdf (true/false)
     */
    bool autoloadPdfXoj{};

    /**
     * Automatically load most recent document on application startup (true/false)
     */
    bool autoloadMostRecent{};

    /**
     * Automatically save documents for crash recovery each x minutes
     */
    int autosaveTimeout{};

    /**
     *  Enable automatic save
     */
    bool autosaveEnabled{};

    /**
     * Allow scroll outside the page display area (horizontal)
     */
    bool addHorizontalSpace{};

    /**
     * How much allowance to scroll outside the page display area (either side of )
     */
    int addHorizontalSpaceAmount{};

    /**
     * Allow scroll outside the page display area (vertical)
     */
    bool addVerticalSpace{};

    /** How much allowance to scroll outside the page display area (above and below)
     */
    int addVerticalSpaceAmount{};

    /**
     * Emulate modifier keys based on initial direction of drawing tool ( for Rectangle, Ellipse etc. )
     */
    bool drawDirModsEnabled{};

    /**
     * Radius at which emulated modifiers are locked on for the rest of drawing operation
     */
    int drawDirModsRadius{};

    /**
     * Rotation snapping enabled by default
     */
    bool snapRotation{};

    /**
     * grid snapping enabled by default
     */
    bool snapGrid{};

    /**
     * Default name if you save a new document
     */
    std::string defaultSaveName;  // should be string - don't change to path

    std::string defaultPdfExportName;

    /**
     * The button config
     */
    std::array<std::unique_ptr<ButtonConfig>, BUTTON_COUNT> buttonConfig;

    /**
     * View-modes. Predefined: 0=default, 1=fullscreen, 2=presentation
     */
    ViewModeId activeViewMode;
    std::vector<ViewMode> viewModes;

    /**
     *  The count of pages which will be cached
     */
    int pdfPageCacheSize{};

    /**
     *  Percentage by which the page's zoom must change
     * for PDF pages to re-render while zooming.
     */
    double pageRerenderThreshold{};

    /**
     * Don't start zooming with touch until the difference in distances between the
     * current touch points and the original is greater than this percentage of the
     * original distance.
     */
    double touchZoomStartThreshold{};

    /**
     * The color to draw borders on selected elements
     * (Page, insert image selection etc.)
     */
    Color selectionBorderColor{};

    /**
     * Color for Text selection, Stroke selection etc.
     */
    Color selectionMarkerColor{};

    /**
     * The color for Xournal page background
     */
    Color backgroundColor{};

    /**
     * Page template String
     */
    std::string pageTemplate;

    /**
     * Unit, see XOJ_UNITS
     */
    std::string sizeUnit;

    /**
     * Audio folder for audio recording
     */
    fs::path audioFolder;

    /**
     * Snap tolerance for the graph/dotted grid
     */
    double snapGridTolerance{};

    /**
     * Rotation epsilon for rotation snapping feature
     */
    double snapRotationTolerance{};

    /// Grid size for Snapping
    double snapGridSize{};

    /**
     * Minimum size of stroke to detect shape
     */
    double strokeRecognizerMinSize{};

    /// Touchscreens act like multi-touch-aware pens.
    bool touchDrawing{};

    /// True iff we use GTK's built-in kinetic/inertial scrolling
    /// for touchscreen devices. If false, we use our own.
    bool gtkTouchInertialScrolling{};

    /**
     * Infer pressure from speed when device pressure
     * is unavailable (e.g. drawing with a mouse).
     */
    bool pressureGuessing{};

    /**
     * The index of the audio device used for recording
     */
    PaDeviceIndex audioInputDevice{};

    /**
     * The index of the audio device used for playback
     */
    PaDeviceIndex audioOutputDevice{};

    /**
     * The sample rate used for recording
     */
    double audioSampleRate{};

    /**
     * The gain by which to amplify the recorded audio samples
     */
    double audioGain{};

    /**
     * The default time by which the playback will seek backwards and forwards
     */
    unsigned int defaultSeekTime{};

    /**
     * List of enabled plugins (only the one which are not enabled by default)
     */
    std::string pluginEnabled;

    /**
     * List of disabled plugins (only the one which are not disabled by default)
     */
    std::string pluginDisabled;


    /**
     * Used to filter strokes of short time and length unless successive in order to do something else ( i.e. select
     * object, float Toolbox menu ). strokeFilterIgnoreLength			this many mm ( double ) strokeFilterIgnoreTime
     * within this time (ms)  will be ignored.. strokeFilterSuccessiveTime		...unless successive within this time.
     */
    int strokeFilterIgnoreTime{};
    double strokeFilterIgnoreLength{};
    int strokeFilterSuccessiveTime{};
    bool strokeFilterEnabled{};
    bool doActionOnStrokeFiltered{};
    bool trySelectOnStrokeFiltered{};

    /**
     * Whether snapping for recognized shapes is enabled
     */
    bool snapRecognizedShapesEnabled{};

    /**
     * Whether the line width should be preserved in a resizing operation
     */
    bool restoreLineWidthEnabled{};

    /**
     * How many stylus events since hitting the screen should be ignored before actually starting the action. If set to
     * 0, no event will be ignored. Should not be negative.
     */
    int numIgnoredStylusEvents{};

    /**
     * Whether Wacom parameter TabletPCButton is enabled
     */
    bool inputSystemTPCButton{};

    bool inputSystemDrawOutsideWindow{};

    std::map<std::string, std::pair<InputDeviceTypeOption, GdkInputSource>> inputDeviceClasses = {};

    /**
     * "Transaction" running, do not save until the end is reached
     */
    bool inTransaction{};

    /** The preferred locale as its language code
     * e.g. "en_US"
     */
    std::string preferredLocale;

    /**
     * The number of pages to pre-load before the current page.
     */
    unsigned int preloadPagesBefore{};

    /**
     * The number of pages to pre-load after the current page.
     */
    unsigned int preloadPagesAfter{};

    /**
     * Whether to evict from the page buffer cache when scrolling.
     */
    bool eagerPageCleanup{};

    /**
     * Stabilizer related settings
     */
    bool stabilizerCuspDetection{};
    bool stabilizerFinalizeStroke{};

    size_t stabilizerBuffersize{};
    double stabilizerDeadzoneRadius{};
    double stabilizerDrag{};
    double stabilizerMass{};
    double stabilizerSigma{};
    StrokeStabilizer::AveragingMethod stabilizerAveragingMethod{};
    StrokeStabilizer::Preprocessor stabilizerPreprocessor{};

    /**
     * @brief Color Palette for tool colors
     *
     */
    std::unique_ptr<Palette> palette;
};
