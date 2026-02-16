#include "FlatPdfCache.h"

#include <algorithm>  // for sort
#include <chrono>
#include <utility>  // for move

#include <glib.h>  // for g_warning

#include "pdf/base/XojPdfDocument.h"  // for XojPdfDocument
#include "util/Assert.h"              // for xoj_assert
#include "util/Range.h"               // for Range
#include "view/Mask.h"                // for Mask

using namespace xoj::view;

class FlatPdfCache::Entry final {
public:
    using Clock = std::chrono::steady_clock;
    Entry(XojPdfPageSPtr popplerPage, Mask&& buffer): popplerPage(std::move(popplerPage)), buffer(std::move(buffer)) {}

    ~Entry() = default;

    XojPdfPageSPtr popplerPage;
    Mask buffer;
    Clock::time_point lastUsed;
};

FlatPdfCache::FlatPdfCache(const XojPdfDocument& doc): pdfDocument(doc), sizeInkB(0) {}

FlatPdfCache::~FlatPdfCache() = default;

auto FlatPdfCache::lookup(size_t pdfPageNo) const -> Entry* {
    for (auto& e: this->entries) {
        if (static_cast<size_t>(e->popplerPage->getPageId()) == pdfPageNo) {
            return e.get();
        }
    }

    return nullptr;
}

auto FlatPdfCache::cache(XojPdfPageSPtr popplerPage, Mask&& buffer) -> Entry* {
    static constexpr int MAX_SIZE_IN_MB = 10;  // Should be more than enough. 1 A4 page in the sidebar should be < 50kB
    sizeInkB += buffer.getMemorySize() / 1024;
    if (sizeInkB > 1024 * MAX_SIZE_IN_MB) {
        // Most recently used first
        std::sort(entries.begin(), entries.end(),
                  [](const auto& e1, const auto& e2) { return e1->lastUsed > e2->lastUsed; });
        auto it = entries.rbegin();
        for (; it < entries.rend(); ++it) {
            auto s = (*it)->buffer.getMemorySize() / 1024;
            if (sizeInkB > 900 * MAX_SIZE_IN_MB) {  // We clear an extra 10% to avoid clearing every time
                xoj_assert(sizeInkB >= s);
                sizeInkB -= s;
            } else {
                break;
            }
        }
        entries.erase(it.base(), entries.end());
    }

    return entries.emplace_back(std::make_unique<Entry>(std::move(popplerPage), std::move(buffer))).get();
}

void FlatPdfCache::paint(cairo_t* cr, size_t pdfPageNo, double pageWidth, double pageHeight) {
    // get zoom from cairo
    cairo_matrix_t matrix = {0};
    cairo_get_matrix(cr, &matrix);
    xoj_assert(matrix.xx == matrix.yy && matrix.xy == 0.0 && matrix.yx == 0.0);  // Homothety matrix
    double zoom = matrix.xx;

    static constexpr double ZOOM_CHANGE_THRESHOLD = 0.05;  ///< 5% change in zoom triggers an update of the cached mask
    std::lock_guard<std::mutex> lock(this->renderMutex);

    Entry* cacheResult = lookup(pdfPageNo);

    if (!cacheResult || std::abs(cacheResult->buffer.getZoom() / zoom - 1.) > ZOOM_CHANGE_THRESHOLD) {
        auto popplerPage = cacheResult ? cacheResult->popplerPage : pdfDocument.getPage(pdfPageNo);

        if (cacheResult) {
            // Wrong zoom - remove the entry
            sizeInkB -= cacheResult->buffer.getMemorySize() / 1024;
            std::erase_if(entries, [cacheResult](const auto& p) { return p.get() == cacheResult; });
        }

        if (!popplerPage) {
            renderMissingPdfPage(cr, pdfPageNo, pageWidth, pageHeight);
            return;
        }

        Mask buffer(cairo_get_target(cr), Range(0, 0, popplerPage->getWidth(), popplerPage->getHeight()), zoom,
                    CAIRO_CONTENT_COLOR_ALPHA);
        popplerPage->render(buffer.get());
        cacheResult = cache(popplerPage, std::move(buffer));
    }

    cacheResult->lastUsed = Entry::Clock::now();
    cacheResult->buffer.paintTo(cr);
}
