/*
 * Xournal++
 *
 * Template class for export classes (e.g. PDF/Image export)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <map>       // for map
#include <memory>    // for std::unique_ptr
#include <optional>  // for std::optional

#include "model/Layer.h"              // for Layer
#include "model/PageRef.h"            // for PageRef
#include "util/ElementRange.h"        // for ElementRangeVector
#include "util/raii/CairoWrappers.h"  // for CairoSurfaceSPtr, CairoSPtr

#include "BaseExportJob.h"  // for ExportBackgroundType, EXPORT_BACKGROUND_ALL

class Document;
class ProgressListener;

/**
 * @brief Template class for export classes
 */
class ExportTemplate {
public:
    ExportTemplate(Document* doc, fs::path filePath);

    virtual ~ExportTemplate();

    void setExportBackground(const ExportBackgroundType exportBackground);

    void setProgressListener(ProgressListener* progressListener);

    void setExportRange(const PageRangeVector& exportRange);
    void setExportRange(const char* rangeStr);

    void setLayerRange(const LayerRangeVector& layerRange);
    void setLayerRange(const char* rangeStr);

    void setProgressiveMode(const bool progressiveMode);

    /**
     * @brief Get the last error message
     * @return The last error message to show to the user
     */
    auto getLastErrorMsg() const -> std::optional<std::string>;

    /**
     * @brief Export document template method
     * @return Success/failure indicator
     */
    void exportDocument();

protected:
    /**
     * Document to export
     */
    Document* doc;

    /**
     * Filename for export
     */
    const fs::path filePath;

    /**
     * The last error message to show to the user
     */
    std::optional<std::string> lastError = std::nullopt;

    /**
     * Cairo export surface
     */
    xoj::util::CairoSurfaceSPtr surface;

    /**
     * Cairo context
     */
    xoj::util::CairoSPtr cr;

    /**
     * The number of the pages to be exported
     */
    size_t numberOfPagesToExport = 0;

private:
    /**
     * @brief Create Cairo cr and surface for a given page
     * @param width the width of the page being exported
     * @param height the height of the page being exported
     *
     * @return true if surface creation succeeded
     */
    virtual auto createCairoResources(int width, int height) -> bool = 0;

    /**
     * @brief Configure Cairo Resources for page to export
     * @return true on successful configuration
     */
    virtual auto configureCairoResourcesForPage(const size_t page) -> bool = 0;

    /**
     * @brief Clear current Cairo configuration
     * @return true on success
     */
    virtual void clearCairoConfig() = 0;

    /**
     * @brief Export all pages in a RangeEntry
     * @param exportedPages number of already exported pages
     * @return Number of pages from the RangeEntry
     */
    auto exportPagesInRangeEntry(const ElementRangeEntry& rangeEntry, const size_t exportedPages) -> size_t;

    /**
     * @brief Export all layers of a single page one by one (each layer a new page)
     * @return true on successful export
     */
    void exportPageLayersProgressivelyForPage(const size_t pageNo);

    /**
     * @brief Export a single page
     * @return true on successful export
     */
    auto exportPage(const size_t pageNo) -> bool;

    /**
     * @brief Render page Background if available and exportBackground set
     */
    void renderBackground(const PageRef& page);

    /**
     * @brief Draw the page
     */
    void drawPage(const PageRef& page);

    /**
     * Option for background export
     */
    ExportBackgroundType exportBackground = EXPORT_BACKGROUND_ALL;

    /**
     * A listener to track the export progress
     */
    ProgressListener* progressListener = nullptr;

    /**
     * The page range to export
     */
    PageRangeVector exportRange;

    /**
     * A pointer to a range of layers to export (the same for every exported pages)
     */
    std::optional<LayerRangeVector> layerRange = std::nullopt;

    /**
     * Export all Layers progressively
     */
    bool progressiveMode = false;
};

/**
 * @brief Parses the provided range string
 * @param rangeStr String specifying the a page or layer range
 *
 * @return nullopt if no or empty range, otherwise ElementRangeVector
 */
auto parseRange(const char* rangeStr) -> std::optional<LayerRangeVector>;

/**
 * @brief Counts the number of pages to export
 * @return total number of pages to export
 */
auto countPagesToExport(const PageRangeVector& exportRange) -> size_t;


/**
 * @brief Clear the page's layers visibility state (all set to false)
 * @return The previous layer visibility state (before cleared)
 */
auto clearLayerVisibilityStateOfPage(const PageRef& page) -> std::map<Layer*, bool>;

/**
 * @brief Set the page's layer visibility state
 */
void setLayerVisibilityStateOfPage(const PageRef& page, std::map<Layer*, bool> visibilityState);
