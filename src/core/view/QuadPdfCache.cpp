#include "QuadPdfCache.h"

#include <algorithm>  // for max
#include <cmath>      // for ceil, abs
#include <cstdio>     // for size_t
#include <memory>     // for shared_ptr, __shared_ptr_access
#include <string>     // for string
#include <utility>    // for move

#include <glib.h>  // for g_warning

#include "control/settings/Settings.h"  // for Settings
#include "pdf/base/XojPdfDocument.h"    // for XojPdfDocument
#include "util/Range.h"                 // for Range
#include "util/i18n.h"                  // for _
#include "util/safe_casts.h"            // for as_unsigned
#include "view/Mask.h"                  // for Mask
#include "view/QuadCache.h"             // for QuadCache

using namespace xoj::view;

struct QuadPdfCache::Entry {
    Entry(XojPdfPageSPtr pdfPage, int DPI):
            popplerPage(std::move(pdfPage)),
            buffer(Range(0., 0., popplerPage->getWidth(), popplerPage->getHeight()), DPI, [this](auto& node) {
                auto mask = this->buffer.makeSuitableMask({node.getArea(), node.getDepth()});
                this->popplerPage->render(mask->get());
                node.assignBuffer(std::move(mask));
                return true;
            }) {}
    XojPdfPageSPtr popplerPage;
    QuadCache buffer;

    inline QuadCache* getCache() { return &buffer; }
};

// TODO put right settings
QuadPdfCache::QuadPdfCache(const XojPdfDocument& doc, Settings* settings):
        pdfDocument(doc), maxMemoryUsageInMB(settings->getMaxViewBufferMemoryUsage()) {}

QuadPdfCache::~QuadPdfCache() = default;

void QuadPdfCache::setMaxMemoryUsage(unsigned int usage) {
    this->maxMemoryUsageInMB = usage;
    prune();
}

void QuadPdfCache::prune() {
    std::lock_guard<std::mutex> lock(this->renderMutex);
    QuadCache::pruneGroup(this->data, maxMemoryUsageInMB);
}

auto QuadPdfCache::lookup(size_t pdfPageNo) -> Entry* {
    for (auto& e: this->data) {
        if (static_cast<size_t>(e->popplerPage->getPageId()) == pdfPageNo) {
            return e.get();
        }
    }

    auto pdfPage = pdfDocument.getPage(pdfPageNo);
    if (pdfPage) {
        return this->data.emplace_back(std::make_unique<Entry>(pdfDocument.getPage(pdfPageNo), 1)).get();
    }

    return nullptr;
}

void QuadPdfCache::paint(cairo_t* cr, size_t pdfPageNo, double pageWidth, double pageHeight) {
    std::lock_guard<std::mutex> lock(this->renderMutex);
    if (Entry* cacheResult = lookup(pdfPageNo); cacheResult) {
        cacheResult->buffer.paintTo(cr);
    } else {
        renderMissingPdfPage(cr, pdfPageNo, pageWidth, pageHeight);
    }
}

void QuadPdfCache::paintSingleTile(cairo_t* cr, size_t pdfPageNo, const QuadCache::TileInfo& ti, double pageWidth,
                                   double pageHeight) {
    xoj_assert(ti.area.isValid());
    std::lock_guard<std::mutex> lock(this->renderMutex);
    if (Entry* cacheResult = lookup(pdfPageNo); cacheResult) {
        if (cacheResult->popplerPage->getWidth() == pageWidth && cacheResult->popplerPage->getHeight() == pageHeight) {
            cacheResult->buffer.paintSingleTile(cr, ti);
        } else {
            // The page and the PDF background do not have the same size. There is no reason for the given TileInfo to
            // correspond to a single tile of the QuadPdfCache: fallback to a normal paint()
            cacheResult->buffer.paintTo(cr);
        }
    } else {
        renderMissingPdfPage(cr, pdfPageNo, pageWidth, pageHeight);
    }
}
