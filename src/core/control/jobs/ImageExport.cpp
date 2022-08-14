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


ImageExport::ImageExport(Document* doc, fs::path filePath, ExportGraphicsFormat format):
        ExportTemplate{doc, std::move(filePath)}, format(format) {}

ImageExport::~ImageExport() = default;

auto ImageExport::setQualityParameter(const RasterImageQualityParameter& qParam) -> void { qualityParameter = qParam; }

auto ImageExport::setQualityParameter(ExportQualityCriterion criterion, int value) -> void {
    this->qualityParameter = RasterImageQualityParameter(criterion, value);
}

auto ImageExport::configureCairoResourcesForPage(const PageRef page) -> bool {
    if (!createCairoCr(page->getWidth(), page->getHeight())) {
        lastError = _("Error: cannot configure Cairo resources.");
        return false;
    }
    return true;
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
