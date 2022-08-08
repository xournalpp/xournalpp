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

#include <memory>  // for std::unique_ptr

#include "util/ElementRange.h"  // for LayerRangeVector

#include "BaseExportJob.h"  // for ExportBackgroundType, EXPORT_BACKGROUND_ALL

class Document;
class ProgressListener;

/**
 * @brief Template class for export classes
 */
class ExportTemplate {
public:
    ExportTemplate(Document* doc, ExportBackgroundType exportBackground, ProgressListener* progressListener,
                   fs::path filePath, const PageRangeVector& exportRange);

    virtual ~ExportTemplate();

    /**
     * @brief Select layers to export by parsing str
     * @param rangeStr A string parsed to get a list of layers
     */
    auto setLayerRange(const char* rangeStr) -> void;

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

protected:
    /**
     * A pointer to a range of layers to export (the same for every exported pages)
     */
    std::unique_ptr<LayerRangeVector> layerRange;

    /**
     * The page range to export
     */
    const PageRangeVector& exportRange;

    /**
     * The last error message to show to the user
     */
    std::string lastError;

    /**
     * Document to export
     */
    Document* doc = nullptr;

    /**
     * Option for background export
     */
    ExportBackgroundType exportBackground = EXPORT_BACKGROUND_ALL;

    /**
     * A listener to track the export progress
     */
    ProgressListener* progressListener = nullptr;

    /**
     * Cairo export surface
     */
    cairo_surface_t* surface = nullptr;

    /**
     * Cairo context
     */
    cairo_t* cr = nullptr;

    /**
     * Filename for export
     */
    fs::path const filePath;

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
     * @brief Export a single page
     * @return true on successful export
     */
    virtual auto exportPage(const size_t pageNo) -> bool = 0;
};
