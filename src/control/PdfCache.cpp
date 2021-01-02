#include "PdfCache.h"

#include <algorithm>
#include <cstdio>
#include <utility>

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

PdfCache::PdfCache(int size) {
    this->size = size;

    g_mutex_init(&this->renderMutex);
}

PdfCache::~PdfCache() {
    clearCache();
    this->size = 0;
}

void PdfCache::setZoom(double zoom) { this->zoom = zoom; }

void PdfCache::setRefreshThreshold(double threshold) { this->zoomRefreshThreshold = threshold; }

void PdfCache::setAnyZoomChangeCausesRecache(bool b) { this->zoomClearsCache = b; }

void PdfCache::clearCache() {
    for (PdfCacheEntry* e: this->data) {
        delete e;
    }
    this->data.clear();
}

auto PdfCache::lookup(const XojPdfPageSPtr& popplerPage) -> PdfCacheEntry* {
    for (PdfCacheEntry* e: this->data) {
        if (e->popplerPage->getPageId() == popplerPage->getPageId()) {
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

void PdfCache::render(cairo_t* cr, const XojPdfPageSPtr& popplerPage, double zoom) {
    g_mutex_lock(&this->renderMutex);

    this->setZoom(zoom);

    PdfCacheEntry* cacheResult = lookup(popplerPage);
    bool needsRefresh{cacheResult == nullptr};

    if (cacheResult != nullptr) {
        double averagedZoom = (this->zoom + cacheResult->zoom) / 2.0;
        double percentZoomChange = std::abs(cacheResult->zoom - this->zoom) * 100.0 / averagedZoom;

        // If we do have a cached result, is its rendering quality
        // acceptable for our current zoom?
        needsRefresh = this->zoom > 1.0 && percentZoomChange > this->zoomRefreshThreshold

                       // Has the user requested that we **always** clear the cache on zoom?
                       || this->zoomClearsCache && this->zoom != cacheResult->zoom;
    }

    if (needsRefresh) {
        double renderZoom = std::max(zoom, 1.0);

        auto* img = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, popplerPage->getWidth() * renderZoom,
                                               popplerPage->getHeight() * renderZoom);
        cairo_t* cr2 = cairo_create(img);

        cairo_scale(cr2, renderZoom, renderZoom);
        popplerPage->render(cr2, false);
        cairo_destroy(cr2);

        cacheResult = cache(popplerPage, img, renderZoom);
    }

    cairo_matrix_t mOriginal;
    cairo_matrix_t mScaled;
    cairo_get_matrix(cr, &mOriginal);
    cairo_get_matrix(cr, &mScaled);
    mScaled.xx = this->zoom / cacheResult->zoom;
    mScaled.yy = this->zoom / cacheResult->zoom;
    mScaled.xy = 0;
    mScaled.yx = 0;
    cairo_set_matrix(cr, &mScaled);
    cairo_set_source_surface(cr, cacheResult->rendered, 0, 0);
    cairo_paint(cr);
    cairo_set_matrix(cr, &mOriginal);

    g_mutex_unlock(&this->renderMutex);
}
