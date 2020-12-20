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

#include <string>
#include <vector>

#include <gtk/gtk.h>

#include "view/DocumentView.h"

#include "BaseExportJob.h"
#include "PageRange.h"
#include "XournalType.h"
#include "filesystem.h"

class Document;
class ProgressListener;

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
    ExportQualityCriterion getQualityCriterion();

    /**
     * @brief Get the target value of this parameter
     * @return The target value
     */
    int getValue();

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
class ImageExport {
public:
    ImageExport(Document* doc, fs::path file, ExportGraphicsFormat format, ExportBackgroundType exportBackground,
                PageRangeVector& exportRange);
    virtual ~ImageExport();

public:
    /**
     * @brief Get the last error message
     * @return The last error message to show to the user
     */
    string getLastErrorMsg() const;

    /**
     * @brief Create one Graphics file per page
     * @param stateListener A listener to track the progress
     */
    void exportGraphics(ProgressListener* stateListener);

    /**
     * @brief Set a quality level for PNG exports
     * @param qParam A quality parameter for the export
     */
    void setQualityParameter(RasterImageQualityParameter qParam);

    /**
     * @brief Set a quality level for PNG exports
     * @param criterion A quality criterion for the export
     * @param value The target value of this criterion
     */
    void setQualityParameter(ExportQualityCriterion criterion, int value);

private:
    /**
     * @brief Create Cairo surface for a given page
     * @param width the width of the page being exported
     * @param height the height of the page being exported
     * @param id the id of the page being exported
     * @param zoomRatio the zoom ratio for PNG exports with fixed DPI
     *
     * @return the zoom ratio of the current page if the export type is PNG, 0.0 otherwise
     *          The return value may differ from that of the parameter zoomRatio
     *          if the export has fixed page width or height (in pixels)
     */
    double createSurface(double width, double height, int id, double zoomRatio);

    /**
     * Free / store the surface
     */
    bool freeSurface(int id);

    /**
     * @brief Get a filename with a (page) number appended
     * @param no The appended number. If no==-1, does not append anything.
     * e.g. .../export-2.png, if the no is -1, return .../export.png
     *
     * @return The filename
     */
    fs::path getFilenameWithNumber(int no) const;

    /**
     * @brief Export a single PNG/SVG page
     * @param pageId The index of the page being exported
     * @param id The number of the page being exported
     * @param zoomRatio The zoom ratio for PNG exports with fixed DPI
     * @param format The format of the exported image
     * @param view A DocumentView for drawing the page
     */
    void exportImagePage(int pageId, int id, double zoomRatio, ExportGraphicsFormat format, DocumentView& view);

public:
    /**
     * Document to export
     */
    Document* doc = nullptr;

    /**
     * Filename for export
     */
    fs::path file;

    /**
     * Export graphics format
     */
    ExportGraphicsFormat format = EXPORT_GRAPHICS_UNDEFINED;

    /**
     * Do not export the Background
     */
    ExportBackgroundType exportBackground = EXPORT_BACKGROUND_ALL;

    /**
     * The range to export
     */
    PageRangeVector& exportRange;

    /**
     * @brief The export quality parameters, used if format==EXPORT_GRAPHICS_PNG
     */
    RasterImageQualityParameter qualityParameter = RasterImageQualityParameter();

    /**
     * Export surface
     */
    cairo_surface_t* surface = nullptr;

    /**
     * Cairo context
     */
    cairo_t* cr = nullptr;

    /**
     * The last error message to show to the user
     */
    string lastError;
};
