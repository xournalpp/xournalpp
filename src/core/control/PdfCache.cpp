#include "PdfCache.h"

#include <algorithm>  // for max
#include <cmath>      // for ceil, abs
#include <cstdio>     // for size_t
#include <memory>     // for shared_ptr, __shared_ptr_access
#include <string>     // for string
#include <utility>    // for move

#include <glib.h>  // for g_warning

#include "control/settings/Settings.h"  // for Settings
#include "pdf/base/XojPdfDocument.h"    // for XojPdfDocument
#include "util/i18n.h"                  // for _

class PdfCacheEntry {
public:
    /**
     *   Cache [img], the result of rendering [popplerPage] with
     * the given [zoom].
     *  A change in the document's zoom causes a change in the
     * quality of the PDF backgrounds (zoomed in => need a higher
     * quality rendering).
     *
     * @param popplerPage
     * @param img is the result of rendering popplerPage
     * @param zoom is the zoom at which the page was rendered.
     */
    PdfCacheEntry(XojPdfPageSPtr popplerPage, cairo_surface_t* img, double zoom) {
        this->popplerPage = std::move(popplerPage);
        this->rendered = img;
        this->zoom = zoom;
    }

    ~PdfCacheEntry() {
        this->popplerPage = nullptr;
        cairo_surface_destroy(this->rendered);
        this->rendered = nullptr;
    }

    double zoom;
    XojPdfPageSPtr popplerPage;
    cairo_surface_t* rendered;
};

PdfCache::PdfCache(const XojPdfDocument& doc, Settings* settings): pdfDocument(doc) { updateSettings(settings); }

PdfCache::~PdfCache() {
    clearCache();
    this->size = 0;
}

void PdfCache::setRefreshThreshold(double threshold) { this->zoomRefreshThreshold = threshold; }

void PdfCache::setMaxSize(size_t newSize) {
    this->size = newSize;
    while (this->data.size() > this->size) {
        delete this->data.back();
        this->data.pop_back();
    }
}

void PdfCache::updateSettings(Settings* settings) {
    if (settings) {
        setMaxSize(settings->getPdfPageCacheSize());
        setRefreshThreshold(settings->getPDFPageRerenderThreshold());
    }
}

void PdfCache::clearCache() {
    for (PdfCacheEntry* e: this->data) { delete e; }
    this->data.clear();
}

auto PdfCache::lookup(size_t pdfPageNo) const -> PdfCacheEntry* {
    for (PdfCacheEntry* e: this->data) {
        if (static_cast<size_t>(e->popplerPage->getPageId()) == pdfPageNo) {
            return e;
        }
    }

    return nullptr;
}

PdfCacheEntry* PdfCache::cache(XojPdfPageSPtr popplerPage, cairo_surface_t* img, double zoom) {
    while (this->data.size() > this->size) {
        delete this->data.back();
        this->data.pop_back();
    }

    auto* ne = new PdfCacheEntry(std::move(popplerPage), img, zoom);
    this->data.push_front(ne);

    return ne;
}

void PdfCache::render(cairo_t* cr, size_t pdfPageNo, double zoom, double pageWidth, double pageHeight) {
    std::lock_guard<std::mutex> lock(this->renderMutex);

    PdfCacheEntry* cacheResult = lookup(pdfPageNo);

    bool needsRefresh = cacheResult == nullptr;

    if (!needsRefresh) {
        double averagedZoom = (zoom + cacheResult->zoom) / 2.0;
        double percentZoomChange = std::abs(cacheResult->zoom - zoom) * 100.0 / averagedZoom;

        // If we do have a cached result, is its rendering quality
        // acceptable for our current zoom?
        needsRefresh = (zoom > 1.0 && percentZoomChange > this->zoomRefreshThreshold);
    }

    if (needsRefresh) {
        double renderZoom = std::max(zoom, 1.0);

        auto popplerPage = cacheResult ? cacheResult->popplerPage : pdfDocument.getPage(pdfPageNo);

        if (!popplerPage) {
            g_warning("PdfCache::render Could not get the pdf page %zu from the document", pdfPageNo);
            renderMissingPdfPage(cr, pageWidth, pageHeight);
            return;
        }

        auto* img = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
                                               static_cast<int>(std::ceil(popplerPage->getWidth() * renderZoom)),
                                               static_cast<int>(std::ceil(popplerPage->getHeight() * renderZoom)));
        /**
         * We can not only rely on cairo_surface_set_device_scale here, as Poppler does not use this scale properly and
         * renders as if 1 pixel = 1 page coordinate unit.
         **/
        cairo_t* cr2 = cairo_create(img);
        cairo_scale(cr2, renderZoom, renderZoom);
        popplerPage->render(cr2);
        cairo_destroy(cr2);

        // but we still use it here to set the scale properly for the upcoming blitting.
        cairo_surface_set_device_scale(img, renderZoom, renderZoom);


        cacheResult = cache(popplerPage, img, renderZoom);
    }

    cairo_set_source_surface(cr, cacheResult->rendered, 0, 0);
    cairo_paint(cr);
}

void PdfCache::renderMissingPdfPage(cairo_t* cr, double pageWidth, double pageHeight) {
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 26);

    cairo_set_source_rgb(cr, 0.8, 0.8, 0.8);

    cairo_text_extents_t extents = {0};
    std::string strMissing = _("PDF background missing");

    cairo_text_extents(cr, strMissing.c_str(), &extents);
    cairo_move_to(cr, pageWidth / 2 - extents.width / 2, pageHeight / 2 - extents.height / 2);
    cairo_show_text(cr, strMissing.c_str());
}
