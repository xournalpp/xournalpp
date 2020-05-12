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

#include <map>
#include <memory>

#include <config-dev.h>
#include <libxml/xmlreader.h>
#include <portaudio.h>

#include "control/Tool.h"
#include "model/Font.h"

#include "Path.h"

constexpr auto DEFAULT_GRID_SIZE = 14.17;

enum Buttons {
    BUTTON_ERASER,
    BUTTON_MIDDLE,
    BUTTON_RIGHT,
    BUTTON_TOUCH,
    BUTTON_DEFAULT,
    BUTTON_STYLUS,
    BUTTON_STYLUS2,
    BUTTON_COUNT
};

constexpr auto buttonToString(Buttons button) -> const char* {
    switch (button) {
        case BUTTON_ERASER:
            return "eraser";
        case BUTTON_MIDDLE:
            return "middle";
        case BUTTON_RIGHT:
            return "right";
        case BUTTON_TOUCH:
            return "touch";
        case BUTTON_DEFAULT:
            return "default";
        case BUTTON_STYLUS:
            return "stylus";
        case BUTTON_STYLUS2:
            return "stylus2";
        default:
            return "unknown";
    }
}

enum AttributeType {
    ATTRIBUTE_TYPE_NONE,
    ATTRIBUTE_TYPE_STRING,
    ATTRIBUTE_TYPE_INT,
    ATTRIBUTE_TYPE_DOUBLE,
    ATTRIBUTE_TYPE_INT_HEX,
    ATTRIBUTE_TYPE_BOOLEAN,
};

// use this as a bit flag
enum ScrollbarHideType {
    SCROLLBAR_HIDE_NONE = 0,
    SCROLLBAR_HIDE_HORIZONTAL = 1 << 1,
    SCROLLBAR_HIDE_VERTICAL = 1 << 2,
    SCROLLBAR_HIDE_BOTH = SCROLLBAR_HIDE_HORIZONTAL | SCROLLBAR_HIDE_VERTICAL
};

/**
 * The user-selectable device types
 */
enum class InputDeviceTypeOption {
    Disabled = 0,
    Mouse = 1,
    Pen = 2,
    Eraser = 3,
    Touchscreen = 4,
    MouseKeyboardCombo = 5,
};

class ButtonConfig;
class InputDevice;


class SAttribute {
public:
    SAttribute();
    SAttribute(const SAttribute& attrib);
    virtual ~SAttribute();

public:
    string sValue;
    int iValue{};
    double dValue{};

    AttributeType type;

    string comment;
};


class SElement final {
    struct SElementData {
    private:
        std::map<string, SAttribute> attributes;
        std::map<string, SElement> children;
        friend class SElement;
    };

public:
    SElement() = default;

    void clear();

    SElement& child(const string& name);

    void setIntHex(const string& name, const int value);
    void setInt(const string& name, const int value);
    void setDouble(const string& name, const double value);
    void setBool(const string& name, const bool value);
    void setString(const string& name, const string& value);

    void setComment(const string& name, const string& comment);

    bool getInt(const string& name, int& value);
    bool getDouble(const string& name, double& value);
    bool getBool(const string& name, bool& value);
    bool getString(const string& name, string& value);

    std::map<string, SAttribute>& attributes();
    std::map<string, SElement>& children();

private:
    std::shared_ptr<SElementData> element = std::make_shared<SElementData>();
};

class Settings {
public:
    /*[[implicit]]*/ Settings(Path filename);
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

    void saveData(xmlNodePtr root, const string& name, SElement& elem);

    void saveButtonConfig();
    void loadButtonConfig();

public:
    // Getter- / Setter
    bool isPressureSensitivity() const;
    void setPressureSensitivity(gboolean presureSensitivity);

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
    void setSelectedToolbar(const string& name);
    string const& getSelectedToolbar() const;

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

    /**
     * The last saved path
     */
    void setLastSavePath(Path p);
    Path const& getLastSavePath() const;

    /**
     * The last open path
     */
    void setLastOpenPath(Path p);
    Path const& getLastOpenPath() const;

    void setLastImagePath(const Path& p);
    Path const& getLastImagePath() const;

    void setMainWndSize(int width, int height);
    void setMainWndMaximized(bool max);
    int getMainWndWidth() const;
    int getMainWndHeight() const;
    bool isMainWndMaximized() const;

    bool isSidebarVisible() const;
    void setSidebarVisible(bool visible);

    int getSidebarWidth() const;
    void setSidebarWidth(int width);

    bool isSidebarOnRight() const;
    void setSidebarOnRight(bool right);

    bool isScrollbarOnLeft() const;
    void setScrollbarOnLeft(bool right);

    bool isMenubarVisible() const;
    void setMenubarVisible(bool visible);

    void setShowPairedPages(bool showPairedPages);
    bool isShowPairedPages() const;

    void setPresentationMode(bool presentationMode);
    bool isPresentationMode() const;

    void setPairsOffset(int numOffset);
    int getPairsOffset() const;

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


    bool isAutloadPdfXoj() const;
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

    bool isTouchWorkaround() const;
    void setTouchWorkaround(bool b);

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

    bool isShowBigCursor() const;
    void setShowBigCursor(bool b);

    bool isHighlightPosition() const;
    void setHighlightPosition(bool highlight);

    uint32_t getCursorHighlightColor() const;
    void setCursorHighlightColor(uint32_t color);

    double getCursorHighlightRadius() const;
    void setCursorHighlightRadius(double radius);

    uint32_t getCursorHighlightBorderColor() const;
    void setCursorHighlightBorderColor(uint32_t color);

    double getCursorHighlightBorderWidth() const;
    void setCursorHighlightBorderWidth(double width);

    ScrollbarHideType getScrollbarHideType() const;
    void setScrollbarHideType(ScrollbarHideType type);

    bool isScrollbarFadeoutDisabled() const;
    void setScrollbarFadeoutDisabled(bool disable);

    string const& getDefaultSaveName() const;
    void setDefaultSaveName(const string& name);

    ButtonConfig* getButtonConfig(int id);

    ButtonConfig* getEraserButtonConfig();
    ButtonConfig* getMiddleButtonConfig();
    ButtonConfig* getRightButtonConfig();
    ButtonConfig* getTouchButtonConfig();
    ButtonConfig* getDefaultButtonConfig();
    ButtonConfig* getStylusButton1Config();
    ButtonConfig* getStylusButton2Config();

    string const& getFullscreenHideElements() const;
    void setFullscreenHideElements(string elements);

    string const& getPresentationHideElements() const;
    void setPresentationHideElements(string elements);

    int getBorderColor() const;
    void setBorderColor(int color);

    int getSelectionColor() const;
    void setSelectionColor(int color);

    int getBackgroundColor() const;
    void setBackgroundColor(int color);

    int getPdfPageCacheSize() const;
    void setPdfPageCacheSize(int size);

    string const& getPageTemplate() const;
    void setPageTemplate(const string& pageTemplate);

    string const& getAudioFolder() const;
    void setAudioFolder(const string& audioFolder);

    PaDeviceIndex getAudioInputDevice() const;
    void setAudioInputDevice(PaDeviceIndex deviceIndex);

    PaDeviceIndex getAudioOutputDevice() const;
    void setAudioOutputDevice(PaDeviceIndex deviceIndex);

    double getAudioSampleRate() const;
    void setAudioSampleRate(double sampleRate);

    double getAudioGain() const;
    void setAudioGain(double gain);

    unsigned int getDefaultSeekTime() const;
    void setDefaultSeekTime(unsigned int t);

    string const& getPluginEnabled() const;
    void setPluginEnabled(const string& pluginEnabled);

    string const& getPluginDisabled() const;
    void setPluginDisabled(const string& pluginDisabled);

    /**
     * Sets #numIgnoredStylusEvents. If given a negative value writes 0 instead.
     */
    void setIgnoredStylusEvents(int numEvents);
    /**
     * Returns #numIgnoredStylusEvents.
     */
    int getIgnoredStylusEvents() const;

    bool getExperimentalInputSystemEnabled() const;
    void setExperimentalInputSystemEnabled(bool systemEnabled);

    bool getInputSystemTPCButtonEnabled() const;
    void setInputSystemTPCButtonEnabled(bool tpcButtonEnabled);

    bool getInputSystemDrawOutsideWindowEnabled() const;
    void setInputSystemDrawOutsideWindowEnabled(bool drawOutsideWindowEnabled);

    void loadDeviceClasses();
    void saveDeviceClasses();
    void setDeviceClassForDevice(GdkDevice* device, InputDeviceTypeOption deviceClass);
    void setDeviceClassForDevice(const string& deviceName, GdkInputSource deviceSource,
                                 InputDeviceTypeOption deviceClass);
    InputDeviceTypeOption getDeviceClassForDevice(GdkDevice* device) const;
    InputDeviceTypeOption getDeviceClassForDevice(const string& deviceName, GdkInputSource deviceSource) const;
    std::vector<InputDevice> getKnownInputDevices() const;

    /**
     * Get name, e.g. "cm"
     */
    string const& getSizeUnit() const;

    /**
     * Get size index in XOJ_UNITS
     */
    int getSizeUnitIndex() const;

    /**
     * Set Unit, e.g. "cm"
     */
    void setSizeUnit(const string& sizeUnit);

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

public:
    // Custom settings
    SElement& getCustomElement(const string& name);

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

private:
    /**
     *  The config filename
     */
    Path filename;

private:
    /**
     * The settings tree
     */
    std::map<string, SElement> data;

    /**
     *  Use pen pressure to control stroke width?
     */
    bool pressureSensitivity{};

    /**
     * If the touch zoom gestures are enabled
     */
    bool zoomGesturesEnabled{};

    /**
     *  If the sidebar is visible
     */
    bool showSidebar{};

    /**
     *  The Width of the Sidebar
     */
    int sidebarWidth{};

    /**
     *  If the sidebar is on the right
     */
    bool sidebarOnRight{};

    /**
     *  Show a better visible cursor for pen
     */
    bool showBigCursor{};

    /**
     * Show a colored circle around the cursor
     */
    bool highlightPosition{};

    /**
     * Cursor highlight color (ARGB format)
     */
    uint32_t cursorHighlightColor{};

    /**
     * Radius of cursor highlight circle. Note that this is limited by the size
     * of the cursor in the display server (default is probably 30 pixels).
     */
    double cursorHighlightRadius{};

    /**
     * Cursor highlight border color (ARGB format)
     */
    uint32_t cursorHighlightBorderColor{};

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
     * If the menu bar is visible on startup
     */
    bool menubarVisible{};

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
    string selectedToolbar;

    /**
     *  The last saved folder
     */
    Path lastSavePath;

    /**
     *  The last opened folder
     */
    Path lastOpenPath;

    /**
     *  The last "insert image" folder
     */
    Path lastImagePath;

    /**
     * The last used font
     */
    XojFont font;

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
    string defaultSaveName;  // should be string - don't change to path

    /**
     * The button config
     */
    ButtonConfig* buttonConfig[BUTTON_COUNT]{};

    /**
     * Which gui elements are hidden if you are in Fullscreen mode,
     * separated by a colon (,)
     */
    string fullscreenHideElements;
    string presentationHideElements;

    /**
     *  The count of pages which will be cached
     */
    int pdfPageCacheSize{};

    /**
     * The color to draw borders on selected elements
     * (Page, insert image selection etc.)
     */
    int selectionBorderColor{};

    /**
     * Color for Text selection, Stroke selection etc.
     */
    int selectionMarkerColor{};

    /**
     * The color for Xournal page background
     */
    int backgroundColor{};

    /**
     * Page template String
     */
    string pageTemplate;

    /**
     * Unit, see XOJ_UNITS
     */
    string sizeUnit;

    /**
     * Audio folder for audio recording
     */
    string audioFolder;

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
     * Do not use GTK Scrolling / Touch handling
     */
    bool touchWorkaround{};

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
    string pluginEnabled;

    /**
     * List of disabled plugins (only the one which are not disabled by default)
     */
    string pluginDisabled;


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
     * How many stylus events since hitting the screen should be ignored before actually starting the action. If set to
     * 0, no event will be ignored. Should not be negative.
     */
    int numIgnoredStylusEvents{};

    /**
     * Whether the new experimental input system is activated
     */
    bool newInputSystemEnabled{};

    /**
     * Whether Wacom parameter TabletPCButton is enabled
     */
    bool inputSystemTPCButton{};

    bool inputSystemDrawOutsideWindow{};

    std::map<string, std::pair<InputDeviceTypeOption, GdkInputSource>> inputDeviceClasses = {};

    /**
     * "Transaction" running, do not save until the end is reached
     */
    bool inTransaction{};
};
