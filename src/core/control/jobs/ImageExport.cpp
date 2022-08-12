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


ImageExport::ImageExport(Document* doc, fs::path filePath, ExportGraphicsFormat format,
                         ExportBackgroundType exportBackground, const PageRangeVector& exportRange,
                         ProgressListener* progressListener):
        ExportTemplate{doc, exportBackground, progressListener, std::move(filePath), exportRange}, format(format) {}

ImageExport::~ImageExport() = default;

/**
 * @brief Set a quality level for PNG exports
 * @param qParam A quality parameter for the export
 */
void ImageExport::setQualityParameter(const RasterImageQualityParameter& qParam) { qualityParameter = qParam; }

/**
 * @brief Set a quality level for PNG exports
 * @param criterion A quality criterion for the export
 * @param value The target value of this criterion
 */
void ImageExport::setQualityParameter(ExportQualityCriterion criterion, int value) {
    this->qualityParameter = RasterImageQualityParameter(criterion, value);
}

auto ImageExport::createCairoCr(double width, double height) -> bool {
    switch (format) {
        case EXPORT_GRAPHICS_PNG:
            switch (qualityParameter.getQualityCriterion()) {
                case EXPORT_QUALITY_WIDTH:
                    zoomRatio = computeZoomRatioWithFactor(width);
                    break;
                case EXPORT_QUALITY_HEIGHT:
                    zoomRatio = computeZoomRatioWithFactor(height);
                    break;
                case EXPORT_QUALITY_DPI:
                    zoomRatio = computeZoomRatioWithFactor(Util::DPI_NORMALIZATION_FACTOR);
                    break;
            }
            width = std::round(width * zoomRatio);
            height = std::round(height * zoomRatio);
            surface =
                    cairo_image_surface_create(CAIRO_FORMAT_ARGB32, static_cast<int>(width), static_cast<int>(height));
            cr = cairo_create(surface);
            cairo_scale(cr, zoomRatio, zoomRatio);
            break;
        case EXPORT_GRAPHICS_SVG:
            surface = cairo_svg_surface_create(getFilenameWithNumber(id).u8string().c_str(), static_cast<int>(width),
                                               static_cast<int>(height));
            cairo_svg_surface_restrict_to_version(surface, CAIRO_SVG_VERSION_1_2);
            cr = cairo_create(surface);
            break;
        default:
            lastError = _("Unsupported graphics format: ") + std::to_string(format);
            return false;
    }
    return cairo_surface_status(surface) == CAIRO_STATUS_SUCCESS;
}

auto ImageExport::computeZoomRatioWithFactor(double normalizationFactor) -> double {
    return ((double)qualityParameter.getValue()) / normalizationFactor;
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
        return filePath;
    }

    auto ext = filePath.extension();
    auto path(filePath);
    path.replace_extension();
    (path += (std::string("-") + std::to_string(no))) += ext;
    return path;
}

auto ImageExport::exportPage(const size_t pageNo) -> bool {
    DocumentView view;

    doc->lock();
    PageRef page = doc->getPage(pageNo);
    doc->unlock();

    if (!configureCairoResourcesForPage(page)) {
        return false;
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

    if (!clearCairoConfig()) {
        return false;
    }

    return true;
}

auto ImageExport::configureCairoResourcesForPage(const PageRef page) -> bool {
    if (!createCairoCr(page->getWidth(), page->getHeight())) {
        lastError = _("Error: cannot configure Cairo resources.");
        return false;
    }
    return true;
}

auto ImageExport::clearCairoConfig() -> bool {
    if (format == EXPORT_GRAPHICS_PNG) {
        const auto filename = getFilenameWithNumber(id);
        cairo_surface_write_to_png(surface, filename.u8string().c_str());
    }

    if (!freeCairoResources()) {
        lastError = _("Error: cannot free Cairo resources.");
        return false;
    }

    return true;
}

RasterImageQualityParameter::RasterImageQualityParameter() = default;
RasterImageQualityParameter::RasterImageQualityParameter(ExportQualityCriterion criterion, int value):
        qualityCriterion{criterion}, value{value} {}
RasterImageQualityParameter::~RasterImageQualityParameter() = default;

auto RasterImageQualityParameter::getQualityCriterion() -> ExportQualityCriterion const { return qualityCriterion; }

auto RasterImageQualityParameter::getValue() -> int const { return value; }
