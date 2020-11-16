#include "ImageExport.h"

#include <utility>

#include <cairo-svg.h>

#include "control/jobs/ProgressListener.h"
#include "model/Document.h"
#include "view/PdfView.h"

#include "Util.h"
#include "i18n.h"


ImageExport::ImageExport(Document* doc, fs::path file, ExportGraphicsFormat format, bool hideBackground,
                         PageRangeVector& exportRange):
        doc(doc), file(std::move(file)), format(format), hideBackground(hideBackground), exportRange(exportRange) {}

ImageExport::~ImageExport() = default;

/**
 * PNG dpi
 */
void ImageExport::setPngDpi(int dpi) { this->pngDpi = dpi; }

/**
 * @return the last error message to show to the user
 */
auto ImageExport::getLastErrorMsg() const -> string { return lastError; }

/**
 * Create surface
 */
void ImageExport::createSurface(double width, double height, int id) {
    if (format == EXPORT_GRAPHICS_PNG) {
        this->surface =
                cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width * this->pngDpi / Util::DPI_NORMALIZATION_FACTOR,
                                           height * this->pngDpi / Util::DPI_NORMALIZATION_FACTOR);
        this->cr = cairo_create(this->surface);
        double factor = this->pngDpi / Util::DPI_NORMALIZATION_FACTOR;
        cairo_scale(this->cr, factor, factor);
    } else if (format == EXPORT_GRAPHICS_SVG) {
        auto filepath = getFilenameWithNumber(id);
        this->surface = cairo_svg_surface_create(filepath.u8string().c_str(), width, height);
        cairo_svg_surface_restrict_to_version(this->surface, CAIRO_SVG_VERSION_1_2);
        this->cr = cairo_create(this->surface);
    } else {
        g_error("Unsupported graphics format: %i", format);
    }
}

/**
 * Free / store the surface
 */
auto ImageExport::freeSurface(int id) -> bool {
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
 * Get a filename with a number, e.g. .../export-1.png, if the no is -1, return .../export.png
 */
auto ImageExport::getFilenameWithNumber(int no) const -> fs::path {
    if (no == -1) {
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
 * Export a single PNG page
 */
void ImageExport::exportImagePage(int pageId, int id, double zoom, ExportGraphicsFormat format, DocumentView& view) {
    doc->lock();
    PageRef page = doc->getPage(pageId);
    doc->unlock();

    createSurface(page->getWidth(), page->getHeight(), id);

    cairo_status_t state = cairo_surface_status(this->surface);
    if (state != CAIRO_STATUS_SUCCESS) {
        this->lastError = _("Error save image #1");
        return;
    }

    if (page->getBackgroundType().isPdfPage()) {
        int pgNo = page->getPdfPageNr();
        XojPdfPageSPtr popplerPage = doc->getPdfPage(pgNo);

        PdfView::drawPage(nullptr, popplerPage, cr, zoom, page->getWidth(), page->getHeight());
    }

    view.drawPage(page, this->cr, true, hideBackground);

    if (!freeSurface(id)) {
        // could not create this file...
        this->lastError = _("Error save image #2");
        return;
    }
}

/**
 * Create one Graphics file per page
 */
void ImageExport::exportGraphics(ProgressListener* stateListener) {
    // don't lock the page here for the whole flow, else we get a dead lock...
    // the ui is blocked, so there should be no changes...
    int count = doc->getPageCount();

    bool onePage =
            ((this->exportRange.size() == 1) && (this->exportRange[0]->getFirst() == this->exportRange[0]->getLast()));

    char selectedPages[count];
    int selectedCount = 0;
    for (int i = 0; i < count; i++) {
        selectedPages[i] = 0;
    }
    for (PageRangeEntry* e: this->exportRange) {
        for (int x = e->getFirst(); x <= e->getLast(); x++) {
            selectedPages[x] = 1;
            selectedCount++;
        }
    }

    stateListener->setMaximumState(selectedCount);

    DocumentView view;
    double zoom = this->pngDpi / Util::DPI_NORMALIZATION_FACTOR;
    int current = 0;

    for (int i = 0; i < count; i++) {
        int id = i + 1;
        if (onePage) {
            id = -1;
        }

        if (selectedPages[i]) {
            stateListener->setCurrentState(current++);

            exportImagePage(i, id, zoom, format, view);
        }
    }
}
