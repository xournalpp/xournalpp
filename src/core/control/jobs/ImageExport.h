/*
 * Xournal++
 *
 * Image export implementation
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cstddef>  // for size_t
#include <string>   // for string

#include <cairo.h>  // for cairo_surface_t, cairo_t

#include "util/ElementRange.h"  // for PageRangeVector, LayerRangeVector

#include "BaseExportJob.h"   // for ExportBackgroundType, EXPORT_BACKGROUND_ALL
#include "ExportTemplate.h"  // for ExportTemplate
#include "filesystem.h"      // for path

class ProgressListener;
class DocumentView;

enum ExportGraphicsFormat { EXPORT_GRAPHICS_UNDEFINED, EXPORT_GRAPHICS_PDF, EXPORT_GRAPHICS_PNG, EXPORT_GRAPHICS_SVG };

/**
 * @brief List of available criterion for determining a PNG export quality.
 * The order must agree with the corresponding listAvailableCriterion in ui/exportSettings.glade
 */
enum ExportQualityCriterion { EXPORT_QUALITY_DPI, EXPORT_QUALITY_WIDTH, EXPORT_QUALITY_HEIGHT };

/**
 * @brief A class storing the available quality parameters for PNG export
 */
class RasterImageQualityParameter {
public:
    RasterImageQualityParameter(ExportQualityCriterion criterion, int value);
    RasterImageQualityParameter();
    ~RasterImageQualityParameter();

    /**
     * @brief Get the quality criterion of this parameter
     * @return The quality criterion
     */
    auto getQualityCriterion() -> ExportQualityCriterion const;

    /**
     * @brief Get the target value of this parameter
     * @return The target value
     */
    auto getValue() -> int const;

private:
    /**
     * @brief Default quality is: DPI=300
     */
    ExportQualityCriterion qualityCriterion = EXPORT_QUALITY_DPI;
    int value = 300;
};

/**
 * @brief A class handling export as images
 */
class ImageExport: public ExportTemplate {
public:
    ImageExport(Document* doc, fs::path filePath, ExportGraphicsFormat format, ExportBackgroundType exportBackground,
                const PageRangeVector& exportRange, ProgressListener* progressListener);
    virtual ~ImageExport();

public:
    /**
     * @brief Create one Graphics file per page
     * @param stateListener A listener to track the progress
     */
    void exportGraphics();

    /**
     * @brief Set a quality level for PNG exports
     * @param qParam A quality parameter for the export
     */
    void setQualityParameter(const RasterImageQualityParameter& qParam);

    /**
     * @brief Set a quality level for PNG exports
     * @param criterion A quality criterion for the export
     * @param value The target value of this criterion
     */
    void setQualityParameter(ExportQualityCriterion criterion, int value);

private:
    auto createCairoCr(double width, double height) -> bool override;

    /**
     * @brief Get a filename with a (page) number appended
     * @param no The appended number. If no==-1, does not append anything.
     * e.g. .../export-2.png, if the no is -1, return .../export.png
     *
     * @return The filename
     */
    fs::path getFilenameWithNumber(size_t no) const;

    /**
     * @brief Export a single PNG/SVG page
     * @param pageId The index of the page being exported
     * @param format The format of the exported image
     * @param view A DocumentView for drawing the page
     */
    void exportImagePage(size_t pageId, ExportGraphicsFormat format, DocumentView& view);

    auto computeZoomRatioWithFactor(double normalizationFactor) -> double;

    static constexpr size_t SINGLE_PAGE = size_t(-1);

public:
    /**
     * Export graphics format
     */
    ExportGraphicsFormat format = EXPORT_GRAPHICS_UNDEFINED;

    /**
     * @brief The export quality parameters, used if format==EXPORT_GRAPHICS_PNG
     */
    RasterImageQualityParameter qualityParameter = RasterImageQualityParameter();

private:
    /**
     * The number of the page being exported
     */
    size_t id = 0;

    /**
     * The zoom ratio for PNG exports with fixed DPI
     */
    double zoomRatio = 1.0;

    auto exportPage(const size_t pageNo) -> bool override;
};
