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
#include "util/Assert.h"                // for xoj_assert
#include "util/Range.h"                 // for Range
#include "util/i18n.h"                  // for _
#include "util/safe_casts.h"            // for as_unsigned
#include "view/Mask.h"                  // for Mask

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
     * @param buffer is the result of rendering popplerPage
     */
    PdfCacheEntry(XojPdfPageSPtr popplerPage, xoj::view::Mask&& buffer):
            popplerPage(std::move(popplerPage)), buffer(std::forward<xoj::view::Mask>(buffer)) {}

    ~PdfCacheEntry() = default;

    XojPdfPageSPtr popplerPage;
    xoj::view::Mask buffer;
};

static double getPercentZoomChange(double oldZoom, double newZoom) {
    double averagedZoom = (oldZoom + newZoom) / 2.0;
    return std::abs(oldZoom - newZoom) * 100.0 / averagedZoom;
}

PdfCache::PdfCache(const XojPdfDocument& doc, Settings* settings): pdfDocument(doc) { updateSettings(settings); }

PdfCache::~PdfCache() = default;

void PdfCache::setRefreshThreshold(double threshold) { this->zoomRefreshThreshold = threshold; }

void PdfCache::setMaxSize(size_t newSize) {
    this->maxSize = newSize;
    if (this->data.size() > this->maxSize) {
        this->data.resize(this->maxSize);
    }
}

void PdfCache::updateSettings(Settings* settings) {
    if (settings) {
        setMaxSize(as_unsigned(settings->getPdfPageCacheSize()));
        setRefreshThreshold(settings->getPDFPageRerenderThreshold());
    }
}

void PdfCache::evictAllExcept(const std::unordered_set<size_t>& retainedPdfPages) {
    std::lock_guard<std::mutex> lock(this->renderMutex);

    for (auto& entry: this->data) {
        xoj_assert(entry);
        xoj_assert(entry->popplerPage);

        const size_t pdfPageNo = static_cast<size_t>(entry->popplerPage->getPageId());
        if (retainedPdfPages.find(pdfPageNo) == retainedPdfPages.end()) {
            entry.reset();
        }
    }

    this->data.erase(std::remove(this->data.begin(), this->data.end(), nullptr), this->data.end());
}

auto PdfCache::lookup(size_t pdfPageNo) const -> const PdfCacheEntry* {
    for (auto& e: this->data) {
        if (static_cast<size_t>(e->popplerPage->getPageId()) == pdfPageNo) {
            return e.get();
        }
    }

    return nullptr;
}

auto PdfCache::cache(XojPdfPageSPtr popplerPage, xoj::view::Mask&& buffer) -> const PdfCacheEntry* {
    xoj_assert(this->maxSize > 0);
    xoj_assert(popplerPage);
    const auto pageId = popplerPage->getPageId();

    auto existingIt = std::find_if(this->data.begin(), this->data.end(),
                                   [pageId](const auto& entry) { return entry->popplerPage->getPageId() == pageId; });
    if (existingIt != this->data.end()) {
        this->data.erase(existingIt);
    }

    if (this->data.size() >= this->maxSize) {
        this->data.resize(this->maxSize - 1);
    }

    this->data.emplace_front(
            std::make_unique<PdfCacheEntry>(std::move(popplerPage), std::forward<xoj::view::Mask>(buffer)));

    return this->data.front().get();
}

void PdfCache::render(cairo_t* cr, size_t pdfPageNo, double zoom, double pageWidth, double pageHeight) {
    std::lock_guard<std::mutex> lock(this->renderMutex);

    const PdfCacheEntry* cacheResult = lookup(pdfPageNo);

    bool needsRefresh = cacheResult == nullptr;

    if (!needsRefresh) {
        // If we do have a cached result, is its rendering quality
        // acceptable for our current zoom?
        needsRefresh =
                (zoom > 1.0 && getPercentZoomChange(cacheResult->buffer.getZoom(), zoom) > this->zoomRefreshThreshold);
    }

    if (needsRefresh) {
        double renderZoom = std::max(zoom, 1.0);

        auto popplerPage = cacheResult ? cacheResult->popplerPage : pdfDocument.getPage(pdfPageNo);

        if (!popplerPage) {
            g_warning("PdfCache::render Could not get the pdf page %zu from the document", pdfPageNo);
            renderMissingPdfPage(cr, pageWidth, pageHeight);
            return;
        }

        xoj::view::Mask buffer(cairo_get_target(cr), Range(0, 0, popplerPage->getWidth(), popplerPage->getHeight()),
                               renderZoom, CAIRO_CONTENT_COLOR_ALPHA);
        popplerPage->render(buffer.get());
        if (this->maxSize == 0) {
            buffer.paintTo(cr);
            return;
        }

        cacheResult = cache(popplerPage, std::move(buffer));
    }

    cacheResult->buffer.paintTo(cr);
}

void PdfCache::renderMissingPdfPage(cairo_t* cr, double pageWidth, double pageHeight) {
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 26);

    cairo_set_source_rgb(cr, 0.8, 0.8, 0.8);

    cairo_text_extents_t extents = {0};
    std::string strMissing = _("PDF background missing");

    cairo_text_extents(cr, strMissing.c_str(), &extents);
    cairo_move_to(cr, pageWidth / 2 - extents.width / 2, pageHeight / 2 - extents.height / 2);
    cairo_text_path(cr, strMissing.c_str());
}