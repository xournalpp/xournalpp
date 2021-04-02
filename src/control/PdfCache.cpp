#include "PdfCache.h"

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <utility>

#include <cairo.h>

// For _1, _2, ...
using namespace std::placeholders;

class PdfCache::CacheEntry: public QuadTreeCache {
public:
    CacheEntry(XojPdfPageSPtr page, const Rect& pageRect, const CacheParams& settings);
    int getPageId() const { return page_->getPageId(); };

    void rerender(cairo_t* cr, const Rect& rect) { page_->render(cr, rect); }

private:
    XojPdfPageSPtr page_;
    Rect pageRect_;
    RenderFn pageRenderFn_;
};

PdfCache::CacheEntry::CacheEntry(XojPdfPageSPtr page, const Rect& pageRect, const CacheParams& settings):
        page_{page}, QuadTreeCache(std::bind(&CacheEntry::rerender, this, _1, _2), pageRect, settings) {
    assert(page_ != nullptr);
}

PdfCache::PdfCache(size_t size) { cacheSettings_.maxSize = size; }

// Need to define a destructor so we can have a unique_ptr
// to a forward-declared class.
PdfCache::~PdfCache() {}

void PdfCache::clear() { data_.clear(); }

auto PdfCache::lookup(XojPdfPageSPtr page) -> std::shared_ptr<PdfCache::CacheEntry> {
    for (const auto& entry: data_) {
        if (entry->getPageId() == page->getPageId()) {
            return entry;
        }
    }

    Rectangle<double> pageRect{0.0, 0.0, page->getWidth(), page->getHeight()};
    std::shared_ptr<CacheEntry> newEntry = std::make_shared<CacheEntry>(page, pageRect, cacheSettings_);

    if (data_.size() > 0) {
        // All entries should, together, have size <= maxSize.
        // Have the new entry contribute to this total.
        newEntry->constrainSizeWith(*data_.front());
    }

    data_.push_back(newEntry);
    return newEntry;
}

void PdfCache::render(cairo_t* cr, const XojPdfPageSPtr& popplerPage, Rectangle<double> srcRegion, double zoom) {
    std::lock_guard lock{renderMutex_};

    Rectangle<double> dstRegion{srcRegion};
    dstRegion *= zoom;

    std::shared_ptr<CacheEntry> entry = lookup(popplerPage);
    entry->render(cr, srcRegion, dstRegion);
}
