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

#include <array>       // for array
#include <cstddef>     // for size_t
#include <functional>  // for function
#include <map>         // for map
#include <memory>      // for make_shared, shared_ptr
#include <string>      // for string, basic_string
#include <utility>     // for pair
#include <vector>      // for vector

#include <gdk/gdk.h>                      // for GdkInputSource, GdkD...
#include <glib.h>                         // for gchar, gboolean, gint
#include <libxml/tree.h>                  // for xmlNodePtr, xmlDocPtr
#include <portaudiocpp/PortAudioCpp.hxx>  // for PaDeviceIndex

#include "control/tools/StrokeStabilizerEnum.h"  // for AveragingMethod, Pre...
#include "model/Font.h"                          // for XojFont
#include "util/Color.h"                          // for Color
#include "util/i18n.h"                           // for _

#include "LatexSettings.h"  // for LatexSettings
#include "SettingsDescription.h"
#include "SettingsEnums.h"  // for InputDeviceTypeOption
#include "ViewModes.h"      // for ViewModes
#include "filesystem.h"     // for path

struct Palette;

constexpr auto DEFAULT_GRID_SIZE = 14.17;

class ButtonConfig;
class InputDevice;


template <class T>
class SettingsContainer {};

template <std::size_t... s>
class SettingsContainer<std::index_sequence<s...>> {
public:
    using params = std::tuple<typename Setting<static_cast<SettingsElement>(s)>::value_type...>;

    SettingsContainer() {
        ((std::get<s>(vars) = Setting<(SettingsElement)s>::DEFAULT), ...);
        ((importFunctions[Setting<(SettingsElement)s>::xmlName] =
                  [this](xmlNodePtr node) { return importSetting<(SettingsElement)s>(node); }),
         ...);
        ((exportFunctions[(int)s] = [this](xmlNodePtr parent) { return exportSetting<(SettingsElement)s>(parent); }),
         ...);
    }

    params vars;
    std::map<std::string, std::function<void(xmlNodePtr)>> importFunctions;
    std::array<std::function<xmlNodePtr(xmlNodePtr)>, (int)SettingsElement::ENUM_COUNT> exportFunctions;
    template <SettingsElement t>
    getter_return_t<typename Setting<t>::value_type> getValue() const {
        return std::get<(std::size_t)t>(vars);
    }
    template <SettingsElement t>
    constexpr const char* getXmlName() const {
        return Setting<t>::xmlName;
    }
    template <SettingsElement t>
    constexpr typename Setting<t>::value_type getDefault() const {
        return Setting<t>::DEFAULT;
    }
    template <SettingsElement t>
    void setValue(const typename Setting<t>::value_type& v) {
        if (!(v == std::get<(std::size_t)t>(vars))) {
            if (validator<t>::enable)
                std::get<(std::size_t)t>(vars) = validator<t>::fn(v);
        }
    }
    template <SettingsElement t>
    void importSetting(xmlNodePtr node) {
        if (importer<t>::fn(node, std::get<(std::size_t)t>(vars))) {
            if (validator<t>::enable)
                std::get<(std::size_t)t>(vars) = validator<t>::fn(std::get<(std::size_t)t>(vars));
        }
    }
    template <SettingsElement t>
    xmlNodePtr exportSetting(xmlNodePtr parent) {
        xmlNodePtr node = exporter<t>::fn(parent, getXmlName<t>(), std::get<(std::size_t)t>(vars));
        const char* com = comment<t>::text;
        if (com != nullptr) {
            auto cNode = xmlNewComment((const xmlChar*)(com));
            xmlAddPrevSibling(node, cNode);
        }
        return node;
    }
};

using SettingsCont = SettingsContainer<std::make_index_sequence<static_cast<std::size_t>(SettingsElement::ENUM_COUNT)>>;

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

    bool getInt(const std::string& name, int& value) const;
    [[maybe_unused]] bool getDouble(const std::string& name, double& value) const;
    bool getBool(const std::string& name, bool& value) const;
    bool getString(const std::string& name, std::string& value) const;

    std::map<std::string, SAttribute>& attributes() const;
    std::map<std::string, SElement>& children() const;

    // This is only used to always update the Setting of type SElement
    // This does not actually check for equality
    bool operator==(const SElement& sel) const { return false; };

private:
    std::shared_ptr<SElementData> element = std::make_shared<SElementData>();
};

class Settings {
public:
    /*[[implicit]]*/ Settings(fs::path filepath);
    Settings(const Settings& settings) = delete;
    void operator=(const Settings& settings) = delete;
    virtual ~Settings();

private:
    SettingsCont settings;

public:
    template <SettingsElement t>
    getter_return_t<typename Setting<t>::value_type> get() const {
        return settings.getValue<t>();
    }

    template <SettingsElement t>
    void set(const typename Setting<t>::value_type& v) {
        settings.setValue<t>(v);
        save();
    }

    bool load();

    void save();

public:
    // View Mode
    bool loadViewMode(ViewModeId mode);

    // Getter- / Setter
    std::vector<ViewMode> getViewModes() const;
    ViewModeId getActiveViewMode() const;

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
    XojFont getFont();
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

    const bool isPageNumberInTitlebarShown() const;
    void setPageNumberInTitlebarShown(const bool shown);

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
    int getAddVerticalSpaceAmountAbove() const;
    void setAddVerticalSpaceAmountAbove(int pixels);
    int getAddVerticalSpaceAmountBelow() const;
    void setAddVerticalSpaceAmountBelow(int pixels);

    bool getAddHorizontalSpace() const;
    void setAddHorizontalSpace(bool space);
    int getAddHorizontalSpaceAmountRight() const;
    void setAddHorizontalSpaceAmountRight(int pixels);
    int getAddHorizontalSpaceAmountLeft() const;
    void setAddHorizontalSpaceAmountLeft(int pixels);

    bool getUnlimitedScrolling() const;
    void setUnlimitedScrolling(bool enable);

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

    bool isAudioDisabled() const;
    void setAudioDisabled(bool disable);

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

    Color getActiveSelectionColor() const;
    void setActiveSelectionColor(Color color);

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

    void setNumberOfSpacesForTab(unsigned int numberSpaces);
    unsigned int getNumberOfSpacesForTab() const;

    void setUseSpacesAsTab(bool useSpaces);
    bool getUseSpacesAsTab() const;

public:
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
     *  The config filepath
     */
    fs::path filepath;

    /**
     * "Transaction" running, do not save until the end is reached
     */
    bool inTransaction{};

    /**
     * @brief Color Palette for tool colors
     *
     */
    std::unique_ptr<Palette> palette;
};
