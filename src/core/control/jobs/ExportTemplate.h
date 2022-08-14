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

#include <memory>    // for std::unique_ptr
#include <optional>  // for std::optional

#include "model/PageRef.h"      // for PageRef
#include "util/ElementRange.h"  // for ElementRangeVector

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
    auto getLastErrorMsg() const -> std::string;

    /**
     * @brief Export document template method
     * @return Success/failure indicator
     */
    auto exportDocument() -> bool;

protected:
    /**
     * @brief Destroy cairo surface and cr
     */
    auto freeCairoResources() -> bool;

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
    std::string lastError;

    /**
     * Cairo export surface
     */
    cairo_surface_t* surface = nullptr;

    /**
     * Cairo context
     */
    cairo_t* cr = nullptr;

private:
    /**
     * @brief Create Cairo cr and surface for a given page
     * @param width the width of the page being exported
     * @param height the height of the page being exported
     *
     * @return true if surface creation succeeded
     */
    virtual auto createCairoCr(double width, double height) -> bool = 0;

    /**
     * @brief Configure Cairo Resources for page to export
     * @return true on successful configuration
     */
    virtual auto configureCairoResourcesForPage(const PageRef page) -> bool = 0;

    /**
     * @brief Clear current Cairo configuration
     * @return true on success
     */
    virtual auto clearCairoConfig() -> bool = 0;

    /**
     * @brief Export all layers of a single page one by one (each layer a new page)
     * @return true on successful export
     */
    auto exportPageLayers(const size_t pageNo) -> bool;

    /**
     * @brief Export a single page
     * @return true on successful export
     */
    auto exportPage(const size_t pageNo) -> bool;

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
