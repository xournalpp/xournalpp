#include "QuadTreeCache.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cmath>
#include <memory>

#include <cairo.h>

using Timestamp = std::chrono::time_point<std::chrono::steady_clock>;

struct CacheState {
    size_t cacheSize = 0;
};

class QuadTreeCache::Node {
public:
    /**
     * @param fn Renders a region from the cache's source.
     * @param region The region in cache coordinates
     *   this node is responsible for.
     * @param cacheSettings is owned by the cache this node belongs
     *   to and specifies cache properties. It should not change after this
     *   is constructed.
     * @param cacheState stores cache-global properties.
     */
    Node(RenderFn fn, const Rect& region, const CacheParams* cacheSettings, std::shared_ptr<CacheState> cacheState);
    ~Node();

    /**
     * @brief Renders this. Should not run concurrently with another render for the
     *   same node.
     * @param cr Context this is drawn onto.
     * @param srcRegion The region in cache-space. May or may not be in this.
     * @param dstRegion The region in screen-space. Should have same aspect ratio
     *   as srcRegion (this may be enforced at runtime).
     * @return true iff srcRegion has a subregion in this.
     */
    bool render(cairo_t* cr, const Rect& srcRegion, const Rect& dstRegion);

    /**
     * @brief Cleans up the cache such that the size of the cache is less than
     *   the target cache size.
     * @param currentZoom Only clear/cleanup nodes with zoom (as by getZoom) greater
     *   than targetZoom.
     */
    void cleanup(double currentZoom);

    /**
     * @param region is marked as needing a redraw.
     */
    void damage(const Rect& region);

    /**
     * @brief Clears cached data associated with this.
     * @return The number of pixels cleared.
     */
    size_t clear();

protected:
    enum QuadIdentifier { TOP_LEFT = 0, TOP_RIGHT = 1, BOT_LEFT = 2, BOT_RIGHT = 3 };

    /**
     * @return How much this is scaled (at most, if scaling differs in this)
     *  to fit source space.
     *  If this has children, returns max({ c.getZoom() : c \in children_ }).
     */
    double getZoom() const;

    /** @return true iff this has associated pixels. */
    bool isEmpty() const;

private:
    /**
     * Splits this into four nodes. Does nothing if already split.
     */
    void divide();

    /**
     * Joins this' children into a single node.
     */
    size_t join();

    /**
     * Sets the timestamp associated with this to now.
     */
    void updateTimestamp();

    /**
     * @brief Render to cr from rendered_.
     * @param cr Context to render to.
     * @param src The rectangle to render from. This must be in region_.
     * @param dst Where src maps to in output coordinates. Must have the same
     *   aspect ratio as src.
     * @return Whether we could render to dst from rendered_.
     */
    bool renderFromSelf(cairo_t* cr, const Rect& src, const Rect& dst);

    /**
     * @brief Clears image data associated with this node until a given number of
     *   pixels have been freed.
     *
     * @param quota Target number of pixels to free.
     * @param targetZoom If a region's internalToSrc_ratio_ (its zoom) is greater than
     *  this, it will be freed.
     * @return The number of pixels freed.
     */
    size_t cleanupByZoom(size_t quota, double targetZoom);
    size_t cleanupByTimestamp(size_t quota);

    /**
     * @brief Clears the rendered image associated directly with this node (and not
     * its children). Does nothing if rendered_ == nullptr.
     * @returns The number of pixels freed by freeing the rendering associated with
     *   this node.
     */
    size_t clearRendered();

    /** @return the number of pixels cleared. */
    size_t clearChildren();

    /** @return true iff this has child nodes. */
    bool hasChildren() const;

    /**
     * @return the number of pixels that would have to be cached for a full render of this.
     */
    size_t getUncachedPx() const;

    /**
     * @param childIdx the index of the child \in [0, 4).
     * @return the rectangle in source space for the child with index, childIdx.
     */
    Rect determineChildRect(size_t childIdx) const;

    /**
     * @param childIdx the index of the child \in [0, 4).
     * @return the rectangle in internal space (relative to rendered_) for the child.
     * @see #determineChildRect for a rectangle in source space.
     */
    Rect getInternalQuadrant(size_t quadrant) const;

    /**
     * Get children of this in decreasing order by zoom.
     */
    std::array<QuadIdentifier, 4> getChildrenByZoom() const;

    /**
     * Get children of this in increasing order by timestamp.
     */
    std::array<QuadIdentifier, 4> getChildrenByTimestamp() const;

private:
    // Renders pixels from the cache's source.
    RenderFn renderFn_;

    // Region in the cache's source this node is responsible for.
    Rect region_;

    // Ptr to settings for the entire cache.
    const CacheParams* cacheSettings_;

    // Cache-global state.
    std::shared_ptr<CacheState> cacheState_;

    // List of nullable children of this. In the order,
    // { TOP_LEFT, TOP_RIGHT, BOTTOM_LEFT, BOTTOM_RIGHT }.
    std::array<std::unique_ptr<Node>, 4> children_;

    // Owned and nullable. Cached render of data managed by this. Must be
    // freed with cairo_surface_destroy.
    cairo_surface_t* rendered_{nullptr};

    // Size of rendered_ image surface (or the size it would be, if it's null).
    size_t internalWidth_, internalHeight_;

    // Conversion factor:
    //  (pixels in rendered_) * internalToSrc_ratio -> (pixels in this->region_)
    double internalToSrc_ratio_;

    // Timestamp of last call to render for this.
    Timestamp lastUsedTimestamp_;
};


QuadTreeCache::QuadTreeCache(RenderFn renderFn, const Rect& pageRect, const CacheParams& cacheSettings):
        cacheSettings_{cacheSettings},
        root_{std::make_unique<Node>(std::move(renderFn), pageRect, &cacheSettings_, std::make_shared<CacheState>())} {}

// We need to define a destructor so root_ can delete itself
// (QuadTreeCache::Node is forward-declared in QuadTreeCache.h).
QuadTreeCache::~QuadTreeCache() {}

void QuadTreeCache::damage(const Rect& region) { root_->damage(region); }

void QuadTreeCache::render(cairo_t* cr, const Rect& srcRegion, const Rect& dstRegion) {
    if (srcRegion.area() == 0.0 || dstRegion.area() == 0.0) {
        return;
    }

    root_->render(cr, srcRegion, dstRegion);

    double currentZoom = dstRegion.width / srcRegion.width;
    root_->cleanup(currentZoom);
}

void QuadTreeCache::clear() { root_->clear(); }


QuadTreeCache::Node::Node(RenderFn fn, const Rect& region, const CacheParams* cacheSettings,
                          std::shared_ptr<CacheState> cacheState):
        renderFn_{std::move(fn)},
        region_{region},
        cacheSettings_{cacheSettings},
        cacheState_{cacheState},
        children_{nullptr, nullptr, nullptr, nullptr} {
    double ratioSquared = static_cast<double>(cacheSettings_->entrySize) / region_.width / region_.height;
    internalToSrc_ratio_ = std::sqrt(ratioSquared);

    internalWidth_ = static_cast<size_t>(region_.width / internalToSrc_ratio_);
    internalHeight_ = static_cast<size_t>(region_.height / internalToSrc_ratio_);

    assert(internalWidth_ * internalHeight_ == cacheSettings_->entrySize);

    updateTimestamp();
}

QuadTreeCache::Node::~Node() { clearRendered(); }

void QuadTreeCache::Node::updateTimestamp() { lastUsedTimestamp_ = std::chrono::steady_clock::now(); }

bool QuadTreeCache::Node::render(cairo_t* cr, const Rect& srcRegion, const Rect& dstRegion) {
    double srcToDst_ratio = dstRegion.width / srcRegion.width;
    std::optional<Rect> srcInThis = region_.intersects(srcRegion);

    // Source and destination regions should have the same aspect ratios.
    assert(std::abs(srcRegion.height * srcToDst_ratio - dstRegion.height) < 0.1);

    // Does srcRegion overlap with this?
    if (!srcInThis.has_value()) {
        return false;
    }

    // Some part of this will be used. As such,
    // we update the last used timestamp.
    updateTimestamp();

    Rect& trimmedSrcRegion = srcInThis.value();
    Rect trimmedDstRegion{dstRegion};

    // Trim the destination region such that it continues to
    // match the source region.
    {
        double dx_src = trimmedSrcRegion.x - srcRegion.x;
        double dy_src = trimmedSrcRegion.y - srcRegion.y;
        double dwidth_src = trimmedSrcRegion.width - srcRegion.width;
        double dheight_src = trimmedSrcRegion.height - srcRegion.height;

        trimmedDstRegion.x += dx_src * srcToDst_ratio;
        trimmedDstRegion.y += dy_src * srcToDst_ratio;
        trimmedDstRegion.width += dwidth_src * srcToDst_ratio;
        trimmedDstRegion.height += dheight_src * srcToDst_ratio;
    }

    if (!renderFromSelf(cr, trimmedSrcRegion, trimmedDstRegion)) {
        return true;
    }

    // Otherwise, we're rendering from children of this.

    // Ensure we have children.
    divide();

    for (const auto& child: children_) { child->render(cr, trimmedSrcRegion, trimmedDstRegion); }

    return true;
}

bool QuadTreeCache::Node::renderFromSelf(cairo_t* cr, const Rect& src, const Rect& dst) {
    if (!rendered_ && hasChildren()) {
        return false;
    }

    double srcToDst_ratio = dst.width / src.width;

    // src and dst should have the same aspect ratio.
    assert(std::abs(src.height * srcToDst_ratio - dst.height) < 0.1);

    // A requirement of renderToSelf is that dst, src \subseteq this->region_.
    // As such,
    double internalToDst_ratio = internalToSrc_ratio_ * srcToDst_ratio;

    // Don't overscale when rendering.
    if (internalToDst_ratio > cacheSettings_->maxZoom) {
        return false;
    }

    cairo_surface_t* rendered = this->rendered_;

    if (!rendered) {
        rendered = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, static_cast<int>(internalWidth_),
                                              static_cast<int>(internalHeight_));
        cairo_t* cr2 = cairo_create(rendered);

        cairo_scale(cr2, 1.0 / internalToSrc_ratio_, 1.0 / internalToSrc_ratio_);
        renderFn_(cr2, region_);
        cairo_destroy(cr2);

        size_t internalSize = internalWidth_ * internalHeight_;
        cacheState_->cacheSize += internalSize;

        this->rendered_ = rendered;
    }

    // Render!
    // TODO: Use a mutex here, if we decide to parallelize rendering.
    cairo_matrix_t mOriginal;
    cairo_matrix_t mScaled;
    cairo_get_matrix(cr, &mOriginal);
    cairo_get_matrix(cr, &mScaled);
    mScaled.xx = internalToDst_ratio;
    mScaled.yy = internalToDst_ratio;
    mScaled.xy = 0;
    mScaled.yx = 0;
    cairo_set_matrix(cr, &mScaled);

    double renderAtX = dst.x / internalToDst_ratio;
    double renderAtY = dst.y / internalToDst_ratio;

    cairo_set_source_surface(cr, rendered, renderAtX, renderAtY);
    cairo_paint(cr);
    cairo_set_matrix(cr, &mOriginal);

    if (rendered != this->rendered_) {
        cairo_surface_destroy(rendered);
        rendered = nullptr;
    }

    return true;
}

size_t QuadTreeCache::Node::getUncachedPx() const {
    // Base case: We don't need to add anything to the cache.
    if (rendered_) {
        return 0;
    }

    // Base case: No children, nothing rendered.
    if (!hasChildren()) {
        return internalWidth_ * internalHeight_;
    }

    size_t result = 0;
    for (size_t i = 0; i < children_.size(); i++) { result += children_[i]->getUncachedPx(); }

    return result;
}

void QuadTreeCache::Node::divide() {
    // Clear cached data associated directly with this.
    clearRendered();

    if (!hasChildren()) {
        for (size_t i = TOP_LEFT; i <= BOT_RIGHT; i++) {
            Rect childRegion = determineChildRect(i);
            children_[i] = std::make_unique<Node>(renderFn_, childRegion, cacheSettings_, cacheState_);
        }

        assert(hasChildren());
    }
}

size_t QuadTreeCache::Node::join() {
    // Must have children to join.
    if (!hasChildren()) {
        return 0;
    }

    // If we'll do less rendering if we just clear this' children,
    if (getUncachedPx() > internalWidth_ * internalHeight_) {
        return clearChildren();
    }

    assert(rendered_ == nullptr);

    size_t bytesFreed = 0;

    rendered_ = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, static_cast<int>(internalWidth_),
                                           static_cast<int>(internalHeight_));
    cairo_t* cr = cairo_create(rendered_);

    for (size_t i = TOP_LEFT; i <= BOT_RIGHT; i++) {
        Node& child = *children_[i];

        child.render(cr, child.region_, getInternalQuadrant(i));
        bytesFreed += child.clear();

        children_[i] = nullptr;
    }

    cairo_destroy(cr);

    assert(bytesFreed > internalWidth_ * internalHeight_);
    bytesFreed -= internalWidth_ * internalHeight_;

    return bytesFreed;
}

size_t QuadTreeCache::Node::clearChildren() {
    if (!hasChildren()) {
        return 0;
    }

    size_t result = 0;

    for (size_t i = 0; i < children_.size(); i++) {
        result += children_[i]->clear();
        children_[i] = nullptr;
    }

    return result;
}

bool QuadTreeCache::Node::hasChildren() const {
    // If one child is null, they all should be.
    // Otherwise, none should be null.
    assert(children_[TOP_LEFT] == children_[TOP_RIGHT] && children_[BOT_LEFT] == children_[BOT_RIGHT] &&
           children_[TOP_RIGHT] == children_[BOT_RIGHT]);

    return children_[TOP_LEFT] != nullptr;
}

QuadTreeCache::Rect QuadTreeCache::Node::determineChildRect(size_t childIdx) const {
    Rect childRegion{region_};
    childRegion.width /= 2.0;
    childRegion.height /= 2.0;

    if (childIdx == TOP_RIGHT || childIdx == BOT_RIGHT) {
        childRegion.x += childRegion.width;
    }

    if (childIdx == BOT_LEFT || childIdx == BOT_RIGHT) {
        childRegion.y += childRegion.height;
    }

    return childRegion;
}

QuadTreeCache::Rect QuadTreeCache::Node::getInternalQuadrant(size_t quadrant) const {
    double quadrantWidth = static_cast<double>(internalWidth_) / 2.0;
    double quadrantHeight = static_cast<double>(internalHeight_) / 2.0;
    double quadrantX = 0.0, quadrantY = 0.0;

    if (quadrant == TOP_RIGHT || quadrant == BOT_RIGHT) {
        quadrantX = quadrantWidth;
    }

    if (quadrant == BOT_LEFT || quadrant == BOT_RIGHT) {
        quadrantY = quadrantHeight;
    }

    return {quadrantX, quadrantY, quadrantWidth, quadrantHeight};
}

void QuadTreeCache::Node::damage(const Rect& region) {
    std::optional<Rect> intersection = region_.intersects(region);

    if (!intersection) {
        return;
    }

    if (hasChildren()) {
        for (int i = 0; i < children_.size(); i++) { children_[i]->damage(region); }
    }

    clearRendered();
}

size_t QuadTreeCache::Node::clearRendered() {
    size_t result = 0;

    if (rendered_ != nullptr) {
        cairo_surface_destroy(rendered_);
        rendered_ = nullptr;

        result = internalWidth_ * internalHeight_;
    }

    assert(cacheState_->cacheSize >= result);
    cacheState_->cacheSize -= result;

    return result;
}

double QuadTreeCache::Node::getZoom() const {
    if (rendered_) {
        return internalToSrc_ratio_;
    }

    double result = 0.0;
    for (size_t i = 0; i < children_.size(); i++) { result = std::max(result, children_[i]->getZoom()); }

    return result;
}

bool QuadTreeCache::Node::isEmpty() const {
    if (hasChildren()) {
        for (size_t i = 0; i < children_.size(); i++) {
            if (!children_[i]->isEmpty()) {
                return false;
            }
        }

        return true;
    } else {
        return rendered_ == nullptr;
    }
}

auto QuadTreeCache::Node::getChildrenByZoom() const -> std::array<QuadTreeCache::Node::QuadIdentifier, 4> {
    assert(hasChildren());

    std::array<QuadIdentifier, 4> result{TOP_LEFT, TOP_RIGHT, BOT_LEFT, BOT_RIGHT};

    std::sort(result.begin(), result.end(), [=](const QuadIdentifier& a, const QuadIdentifier& b) -> bool {
        // Returns true iff a goes before (less than) b.
        return children_[a]->getZoom() > children_[b]->getZoom();
    });

    return result;
}

auto QuadTreeCache::Node::getChildrenByTimestamp() const -> std::array<QuadTreeCache::Node::QuadIdentifier, 4> {
    assert(hasChildren());

    std::array<QuadIdentifier, 4> result{TOP_LEFT, TOP_RIGHT, BOT_LEFT, BOT_RIGHT};

    std::sort(result.begin(), result.end(), [=](const QuadIdentifier& a, const QuadIdentifier& b) -> bool {
        // Returns true iff a should go before b in the result.
        return children_[a]->lastUsedTimestamp_ < children_[b]->lastUsedTimestamp_;
    });

    return result;
}

size_t QuadTreeCache::Node::cleanupByZoom(size_t quota, double targetZoom) {
    if (quota <= 0) {
        return 0;
    }

    size_t result = 0;

    if (hasChildren()) {
        // Children by zoom, decreasing order.
        std::array<QuadIdentifier, 4> childrenByZoom = getChildrenByZoom();

        // Clean up until we reach our quota, or we can't clean up anymore.
        for (size_t i = 0; i < childrenByZoom.size(); i++) {
            QuadIdentifier childId = childrenByZoom[i];
            Node& child = *children_[childId];

            assert(quota >= result);

            if (child.getZoom() > targetZoom) {
                result += child.cleanupByZoom(quota - result, targetZoom);
            } else {
                break;
            }

            if (quota <= result) {
                break;
            }
        }

        // If we can join and downsample this' children while
        // keeping within the target zoom, do so.
        if (getZoom() <= targetZoom && result < quota) {
            result += join();
        }
    }

    if (isEmpty()) {
        clear();
    }

    return result;
}

void QuadTreeCache::Node::cleanup(double currentZoom) {
    // TODO: Add a mutex here for thread safety.
    if (cacheState_->cacheSize > cacheSettings_->maxSize) {
        size_t quota = cacheState_->cacheSize - cacheSettings_->maxSize;

        if (cacheSettings_->uncachePolicy == DecachePolicy::VIEWPORT_THEN_LRU) {
            size_t cleared = cleanupByZoom(quota, currentZoom * cacheSettings_->maxZoom);

            if (cleared >= quota) {
                return;
            }

            quota -= cleared;
        }

        cleanupByTimestamp(quota);
    }
}

size_t QuadTreeCache::Node::cleanupByTimestamp(size_t quota) {
    if (quota <= 0) {
        return 0;
    }

    if (!hasChildren()) {
        return clearRendered();
    }

    size_t result = 0;
    std::array<QuadIdentifier, 4> childrenByTimestamp = getChildrenByTimestamp();

    for (size_t i = 0; i < childrenByTimestamp.size(); i++) {
        result += children_[i]->cleanupByTimestamp(quota - result);

        if (result >= quota) {
            break;
        }
    }

    if (isEmpty()) {
        // If we're empty, we may still need to trim null nodes.
        // clear does this.
        result += clear();
    }

    return result;
}

size_t QuadTreeCache::Node::clear() { return clearRendered() + clearChildren(); }
