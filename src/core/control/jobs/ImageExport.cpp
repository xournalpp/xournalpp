#include "ImageExport.h"

#include <cmath>    // for round
#include <cstddef>  // for size_t
#include <memory>   // for __shared_ptr_access, allocat...
#include <utility>  // for move
#include <vector>   // for vector

#include <cairo-svg.h>  // for cairo_svg_surface_create

#include "control/jobs/BaseExportJob.h"  // for EXPORT_BACKGROUND_NONE, EXPO...
#include "model/Document.h"              // for Document
#include "model/PageRef.h"               // for PageRef
#include "model/PageType.h"              // for PageType
#include "model/XojPage.h"               // for XojPage
#include "pdf/base/XojPdfPage.h"         // for XojPdfPageSPtr, XojPdfPage
#include "util/Util.h"                   // for DPI_NORMALIZATION_FACTOR
#include "util/i18n.h"                   // for _
#include "view/DocumentView.h"           // for DocumentView
#include "view/LayerView.h"
#include "view/View.h"
#include "view/background/BackgroundView.h"

#include "ProgressListener.h"  // for ProgressListener

using std::string;


ImageExport::ImageExport(Document* doc, fs::path file, ExportGraphicsFormat format,
                         ExportBackgroundType exportBackground, const PageRangeVector& exportRange):
        doc(doc), file(std::move(file)), format(format), exportBackground(exportBackground), exportRange(exportRange) {}

ImageExport::~ImageExport() = default;

/**
 * @brief Set a quality level for PNG exports
 * @param qParam A quality parameter for the export
 */
void ImageExport::setQualityParameter(RasterImageQualityParameter qParam) { this->qualityParameter = qParam; }

/**
 * @brief Set a quality level for PNG exports
 * @param criterion A quality criterion for the export
 * @param value The target value of this criterion
 */
void ImageExport::setQualityParameter(ExportQualityCriterion criterion, int value) {
    this->qualityParameter = RasterImageQualityParameter(criterion, value);
}

/**
 * @brief Select layers to export by parsing str
 * @param rangeStr A string parsed to get a list of layers
 */
void ImageExport::setLayerRange(const char* rangeStr) {
    if (rangeStr) {
        // Use no upper bound for layer indices, as the maximal value can vary between pages
        layerRange =
                std::make_unique<LayerRangeVector>(ElementRange::parse(rangeStr, std::numeric_limits<size_t>::max()));
    }
}

/**
 * @brief Get the last error message
 * @return The last error message to show to the user
 */
auto ImageExport::getLastErrorMsg() const -> string { return lastError; }

/**
 * @brief Create Cairo surface for a given page
 * @param width the width of the page being exported
 * @param height the height of the page being exported
 * @param id the id of the page being exported
 * @param zoomRatio the zoom ratio for PNG exports with fixed DPI
 *
 * @return the zoom ratio of the current page if the export type is PNG, 0.0 otherwise
 *          The return value may differ from that of the parameter zoomRatio if the export has fixed page width or
 * height (in pixels). In this case, the zoomRatio (and the DPI) is page-dependent as soon as the document has pages of
 * different sizes.
 */
auto ImageExport::createSurface(double width, double height, size_t id, double zoomRatio) -> double {
    switch (this->format) {
        case EXPORT_GRAPHICS_PNG:
            switch (this->qualityParameter.getQualityCriterion()) {
                case EXPORT_QUALITY_WIDTH:
                    zoomRatio = ((double)this->qualityParameter.getValue()) / width;
                    this->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, this->qualityParameter.getValue(),
                                                               (int)std::round(height * zoomRatio));
                    break;
                case EXPORT_QUALITY_HEIGHT:
                    zoomRatio = ((double)this->qualityParameter.getValue()) / height;
                    this->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, (int)std::round(width * zoomRatio),
                                                               this->qualityParameter.getValue());
                    break;
                case EXPORT_QUALITY_DPI:  // Use the zoomRatio given as argument
                    this->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, (int)std::round(width * zoomRatio),
                                                               (int)std::round(height * zoomRatio));
                    break;
            }
            this->cr = cairo_create(this->surface);
            cairo_scale(this->cr, zoomRatio, zoomRatio);
            return zoomRatio;
        case EXPORT_GRAPHICS_SVG:
            this->surface = cairo_svg_surface_create(getFilenameWithNumber(id).u8string().c_str(), width, height);
            cairo_svg_surface_restrict_to_version(this->surface, CAIRO_SVG_VERSION_1_2);
            this->cr = cairo_create(this->surface);
            break;
        default:
            this->lastError = _("Unsupported graphics format: ") + std::to_string(this->format);
    }
    return 0.0;
}

/**
 * Free / store the surface
 */
auto ImageExport::freeSurface(size_t id) -> bool {
    cairo_destroy(this->cr);

    cairo_status_t status = CAIRO_STATUS_SUCCESS;
    if (format == EXPORT_GRAPHICS_PNG) {
        auto filepath = getFilenameWithNumber(id);
        status = cairo_surface_write_to_png(surface, filepath.u8string().c_str());
    }
    cairo_surface_destroy(surface);

    // we ignore this problem
    return status == CAIRO_STATUS_SUCCESS;
}

/**
 * @brief Get a filename with a (page) number appended
 * @param no The appended number. If no==-1, does not append anything.
 * e.g. .../export-2.png, if the no is -1, return .../export.png
 *
 * @return The filename
 */
auto ImageExport::getFilenameWithNumber(size_t no) const -> fs::path {
    if (no == SINGLE_PAGE) {
        // No number to add
        return file;
    }

    auto ext = file.extension();
    auto path(file);
    path.replace_extension();
    (path += (std::string("-") + std::to_string(no))) += ext;
    return path;
}

/**
 * @brief Export a single PNG/SVG page
 * @param pageId The index of the page being exported
 * @param id The number of the page being exported
 * @param zoomRatio The zoom ratio for PNG exports with fixed DPI
 * @param format The format of the exported image
 * @param view A DocumentView for drawing the page
 */
void ImageExport::exportImagePage(size_t pageId, size_t id, double zoomRatio, ExportGraphicsFormat format,
                                  DocumentView& view) {
    doc->lock();
    PageRef page = doc->getPage(pageId);
    doc->unlock();

    zoomRatio = createSurface(page->getWidth(), page->getHeight(), id, zoomRatio);

    cairo_status_t state = cairo_surface_status(this->surface);
    if (state != CAIRO_STATUS_SUCCESS) {
        this->lastError = _("Error save image #1");
        return;
    }

    if (page->getBackgroundType().isPdfPage() && (exportBackground != EXPORT_BACKGROUND_NONE)) {
        // Handle the pdf page separately, to call renderForPrinting for better quality.
        auto pgNo = page->getPdfPageNr();
        XojPdfPageSPtr popplerPage = doc->getPdfPage(pgNo);
        if (!popplerPage) {
            this->lastError = _("Error while exporting the pdf background: I cannot find the pdf page number ");
            this->lastError += std::to_string(pgNo);
        } else {
            popplerPage->renderForPrinting(cr);
        }
    }

    if (layerRange) {
        view.drawLayersOfPage(*layerRange, page, this->cr, true /* dont render eraseable */,
                              true /* don't rerender the pdf background */, exportBackground == EXPORT_BACKGROUND_NONE,
                              exportBackground <= EXPORT_BACKGROUND_UNRULED);
    } else {
        view.drawPage(page, this->cr, true /* dont render eraseable */, true /* don't rerender the pdf background */,
                      exportBackground == EXPORT_BACKGROUND_NONE, exportBackground <= EXPORT_BACKGROUND_UNRULED);
    }

    if (!freeSurface(id)) {
        // could not create this file...
        this->lastError = _("Error save image #2");
        return;
    }
}

/**
 * @brief Create one Graphics file per page
 * @param stateListener A listener to track the export progress
 */
void ImageExport::exportGraphics(ProgressListener* stateListener) {
    // don't lock the page here for the whole flow, else we get a dead lock...
    // the ui is blocked, so there should be no changes...
    auto count = doc->getPageCount();

    bool onePage = ((this->exportRange.size() == 1) && (this->exportRange[0].first == this->exportRange[0].last));

    std::vector<char> selectedPages(count, 0);
    size_t selectedCount = 0;
    for (PageRangeEntry const& e: this->exportRange) {
        for (size_t x = e.first; x <= e.last; x++) {
            selectedPages[x] = true;
            selectedCount++;
        }
    }

    stateListener->setMaximumState(selectedCount);

    /*
     * Compute the zoomRatio only once if using DPI as a PNG quality criterion
     */
    double zoomRatio = 1.0;
    if ((this->format == EXPORT_GRAPHICS_PNG) && (this->qualityParameter.getQualityCriterion() == EXPORT_QUALITY_DPI)) {
        zoomRatio = ((double)this->qualityParameter.getValue()) / Util::DPI_NORMALIZATION_FACTOR;
    }

    DocumentView view;
    int current = 0;

    for (size_t i = 0; i < count; i++) {
        auto id = i + 1;
        if (onePage) {
            id = SINGLE_PAGE;
        }

        if (selectedPages[i]) {
            stateListener->setCurrentState(current++);

            exportImagePage(i, id, zoomRatio, format, view);  // Todo(narrowing): remove cast
        }
    }
}

RasterImageQualityParameter::RasterImageQualityParameter() = default;
RasterImageQualityParameter::RasterImageQualityParameter(ExportQualityCriterion criterion, int value):
        qualityCriterion(criterion), value(value) {}
RasterImageQualityParameter::~RasterImageQualityParameter() = default;

auto RasterImageQualityParameter::getQualityCriterion() -> ExportQualityCriterion { return qualityCriterion; }

auto RasterImageQualityParameter::getValue() -> int { return value; }
