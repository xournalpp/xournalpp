#include "QuadTreeCache.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cmath>
#include <functional>
#include <iostream>
#include <memory>

#include <cairo.h>

#define DEBUG_QUAD_TREE true

#ifdef DEBUG_QUAD_TREE

#define assertEql(lhs, rhs) {\
    if (lhs != rhs) {\
        std::cout << "QuadTreeCache: Assertion failed: " << lhs << " != " << rhs << std::endl;\
\
        assert(lhs == rhs);\
    }\
}

static void drawDebugRect(cairo_t* cr, unsigned char r, unsigned char g, unsigned char b,
        Rectangle<double> rect, double lineWidth, double alpha = 0.1) {
    cairo_set_source_rgba(cr, r, g, b, alpha);
    cairo_set_line_width(cr, lineWidth);
    cairo_rectangle(cr, rect.x, rect.y, rect.width, rect.height);
    cairo_fill(cr);
    cairo_rectangle(cr, rect.x, rect.y, rect.width, rect.height);
    cairo_set_source_rgb(cr, r, g, b);
    cairo_stroke(cr);
}

#else

#define assertEql(a, b) ;
#define drawDebugRect(cr, r, g, b, rect, lineWidth) ;

#endif

using Timestamp = std::chrono::time_point<std::chrono::steady_clock>;

struct QuadTreeCache::CacheState {
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

    /**
     * @return The number of pixels stored in this (including
     *   this' children).
     */
    size_t getSize() const;

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

protected:
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
     * @return The number of pixels that would need to be cached for a full
     * re-render of this.
     */
    size_t getUncachedPx() const;

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
     * @brief Populates this' and this' childrens' cache from a render.
     * @param render The render's result.
     * @param subregion is the region of render we can use to populate this' cache.
     *        This must have size and position a multiple of (internalWidth_, internalHeight_).
     */
    void cacheFrom(cairo_surface_t* render, Rectangle<size_t> subregion);

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

    /**
     * @param internalToDst_ratio is how much this' internal surface
     *        must be scaled to fit the destination for the render.
     * @return Maximum recursive depth from this for which a call
     *  to renderFn_ will occur if re-rendered.
     *
     * For example, if this node were to force a call to renderFn_,
     * and none of its children were to do the same, the depth would be zero.
     *
     * If this node only had one child force a call to renderFn_, the depth
     * would be 1.
     */
    size_t getRenderCallDepth(double internalToDst_ratio) const;

    /**
     * @param renderForDepth How far down the quad tree we should include in the render.
     * @return Full surface containing a render of this.
     * The caller is responsible for freeing the resultant cairo_surface_t*.
     */
    cairo_surface_t* getRenderFromSourceFn(size_t renderForDepth);

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
        state_{std::make_shared<CacheState>()},
        root_{std::make_unique<Node>(std::move(renderFn), pageRect, &cacheSettings_, state_)},
        lastRenderZoom_{1.0} {}

// We need to define a destructor so root_ can delete itself
// (QuadTreeCache::Node is forward-declared in QuadTreeCache.h).
QuadTreeCache::~QuadTreeCache() {}

void QuadTreeCache::updateSettings(const CacheParams& cacheSettings) {
    std::lock_guard lk{mutex_};

    cacheSettings_ = cacheSettings;
    root_->cleanup(lastRenderZoom_);
}

size_t QuadTreeCache::getCacheSize() const {
    size_t size = state_->cacheSize;
    return size;
}

void QuadTreeCache::constrainSizeWith(QuadTreeCache& other) {
    if (&other == this) {
        return;
    }

    std::scoped_lock lk{other.mutex_, mutex_};

    other.state_->cacheSize += state_->cacheSize;
    state_ = other.state_;
}

void QuadTreeCache::damage(const Rect& region) {
    std::lock_guard lk{mutex_};
    root_->damage(region);
}

void QuadTreeCache::render(cairo_t* cr, const Rect& srcRegion, const Rect& dstRegion) {
    if (srcRegion.area() == 0.0 || dstRegion.area() == 0.0) {
        return;
    }

    std::lock_guard lk{mutex_};

    drawDebugRect(cr, 255, 255, 255, dstRegion, 12, 1.0);

    root_->render(cr, srcRegion, dstRegion);

    drawDebugRect(cr, 100, 0, 255, dstRegion, 12);

    double currentZoom = dstRegion.width / srcRegion.width;
    root_->cleanup(currentZoom);
    lastRenderZoom_ = currentZoom;
}

void QuadTreeCache::clear() {
    std::lock_guard lk{mutex_};

    root_->clear();
}

QuadTreeCache::Node::Node(RenderFn fn, const Rect& region, const CacheParams* cacheSettings,
                          std::shared_ptr<CacheState> cacheState):
        renderFn_{std::move(fn)},
        region_{region},
        cacheSettings_{cacheSettings},
        cacheState_{cacheState},
        children_{nullptr, nullptr, nullptr, nullptr} {
    double ratioSquared = region_.width * region_.height / static_cast<double>(cacheSettings_->entrySize);
    internalToSrc_ratio_ = std::sqrt(ratioSquared);

    std::cout << "region: " << region_.width << "," << region_.height << std::endl;

    internalWidth_ = static_cast<size_t>(region_.width / internalToSrc_ratio_);
    internalHeight_ = static_cast<size_t>(region_.height / internalToSrc_ratio_);

    std::cout << "(" << internalWidth_ << ", " << internalHeight_ << ") to " << cacheSettings_->entrySize << std::endl;
    assert(internalWidth_ * internalHeight_ <= cacheSettings_->entrySize);

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
    if (!srcInThis) {
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

    if (renderFromSelf(cr, trimmedSrcRegion, trimmedDstRegion)) {
        return true;
    }

    // Otherwise, we're rendering from children of this.

    // Ensure we have children.
    divide();

    for (const auto& child: children_) { child->render(cr, srcRegion, dstRegion); }

    drawDebugRect(cr, 0, 0, 255, trimmedDstRegion, 4);
    drawDebugRect(cr, 0, 100, 100, dstRegion, 4);

    return true;
}

cairo_surface_t* QuadTreeCache::Node::getRenderFromSourceFn(size_t renderForDepth) {
    size_t renderMultiplier = static_cast<size_t>(std::pow(2, renderForDepth) / 2);
    size_t surfWidth = internalWidth_ * renderMultiplier;
    size_t surfHeight = internalHeight_ * renderMultiplier;

    if (surfWidth == 0) {
        std::cout << "\x1b[31mSurfwidth is zero!\x1b[0m" << std::endl;
        std::cout << "\tinternalWidth_ = " << internalWidth_ << std::endl
                  << "\trenderMultiplier = " << renderMultiplier << std::endl
                  << "\trenderForDepth = " << renderForDepth << std::endl;
    }

    //     surfWidth * surfToSrc_ratio  := source rectangle width (which is rect_.width)
    // and surfHeight * surfToSrc_ratio := rect_.height
    //
    // By definition, internalWidth_ * internalToSrc_ratio_ = rect_.width, so,
    double surfToSrc_ratio = internalToSrc_ratio_ / static_cast<double>(renderMultiplier);

    cairo_surface_t* rendered =
            cairo_image_surface_create(CAIRO_FORMAT_ARGB32, static_cast<int>(surfWidth), static_cast<int>(surfHeight));
    cairo_t* cr = cairo_create(rendered);

    cairo_scale(cr, 1.0 / surfToSrc_ratio, 1.0 / surfToSrc_ratio);
    cairo_translate(cr, -region_.x, -region_.y);
    renderFn_(cr, region_);
    cairo_destroy(cr);

    cacheFrom(rendered, {0, 0, surfWidth, surfHeight});
    return rendered;
}

void QuadTreeCache::Node::cacheFrom(cairo_surface_t* src, Rectangle<size_t> renderRect) {
    assert(renderRect.width > 0);
    assert(renderRect.height > 0);

    // Recurse.
    if (renderRect.width > internalWidth_ && renderRect.height > internalHeight_) {
        // We need children to recurse!
        divide();

        size_t childRectWidth = renderRect.width / 2;
        size_t childRectHeight = renderRect.height / 2;

        std::cout << "\x1b[32m Recurse: " << childRectWidth << "\x1b[0m" << std::endl;

        for (size_t i = 0; i < children_.size(); i++) {
            Rectangle<size_t> childRenderRect{0, 0, childRectWidth, childRectHeight};

            if (i == TOP_RIGHT || i == BOT_RIGHT)
                childRenderRect.x += childRectWidth;
            if (i == BOT_LEFT || i == BOT_RIGHT)
                childRenderRect.y += childRectHeight;

            children_[i]->cacheFrom(src, childRenderRect);
        }

        std::cout << " \x1b[32m End.\x1b[0m" << std::endl;

        return;
    }

    // If we've already rendered, no need to re-render.
    if (this->rendered_)
        return;

    // Ensure we aren't adding additional children.
    clearChildren();

    assertEql(internalWidth_, renderRect.width);
    assertEql(internalHeight_, renderRect.height);

    this->rendered_ = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, static_cast<int>(internalWidth_),
                                                 static_cast<int>(internalHeight_));

    cairo_t* cr = cairo_create(this->rendered_);
    cairo_set_source_surface(cr, src, -static_cast<double>(renderRect.x), -static_cast<double>(renderRect.y));
    cairo_paint(cr);
    cairo_destroy(cr);

    cacheState_->cacheSize += renderRect.area();
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

    // Get how many children deep we need to render.
    size_t renderDepth = getRenderCallDepth(internalToDst_ratio);

    if (renderDepth > 0) {
        clearRendered();
    }

    cairo_surface_t* rendered = this->rendered_;

    if (!rendered) {
        rendered = getRenderFromSourceFn(renderDepth);

        // Larger renderDepths -> smaller scaling for the (larger) rendered surface.
        for (size_t i = 1; i < renderDepth; i++) internalToDst_ratio /= 2.0;
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

    drawDebugRect(cr, 255, 0, 0, 
        Rectangle<double> {
            renderAtX, renderAtY,
            static_cast<double>(internalWidth_),
            static_cast<double>(internalHeight_)
        },
        3
    );

    cairo_set_matrix(cr, &mOriginal);

    if (rendered != this->rendered_) {
        cairo_surface_destroy(rendered);
        rendered = nullptr;
    }

    return true;
}

size_t QuadTreeCache::Node::getRenderCallDepth(double internalToDst_ratio) const {
    // If we don't need to call render at all,
    if (rendered_ && internalToDst_ratio < cacheSettings_->maxZoom) {
        return 0;
    }

    // Recursive case.
    if (hasChildren()) {
        size_t maxDepth = 0;

        for (size_t i = 0; i < children_.size(); i++) {
            size_t currentDepth = children_[i]->getRenderCallDepth(internalToDst_ratio / 2.0);

            if (currentDepth != 0) {
                // If non-zero, then we'll need to call render; this branch
                // causes a render-call, so we need to increment depth.
                maxDepth = std::max(maxDepth, currentDepth + 1);
            }
            // Conversely, if a branch doesn't lead to a render call (currentDepth == 0),
            // then we shouldn't consider the render call depth to be 1 (we're not adding
            // 1 to currentDepth).
        }

        return maxDepth;
    }

    //  Let m = cacheSettings_->maxZoom, d = internalToDst_ratio,
    //  s = internalToSrc_ratio, and w_int be internalWidth_ and
    // h_int be internalHeight_.
    //  We then have,
    // w_src/2  w_src/2
    //  +-------+
    //  |   |   |
    //  |   |   | h_src / 2
    //  |   |   |
    //  +-------+
    //  |   |   |
    //  |   |   | h_src /2
    //  |   |   |
    //  +-------+
    //
    // Above, we're dividing by 2, but we could subdivide the full box some other number of times.
    // Let the maximum number of times be k.
    //
    // From this, the smallest box has dimensions (w_src / 2**k, h_src / 2**k) and is being mapped onto
    // (w_dst / 2**k, h_dst / 2**k), so is scaled by s * 1.
    //
    // As such, internal space is scaled by d / 2**k (because w_int and h_int are the same for the child
    // as the parent).
    //
    // As such, at maximum, we're scaling by
    //   m  = d / 2**k
    // and so,
    //   m2**k = d
    //   2**k  = d / m
    double minRenderCallDepth =
            std::ceil(std::log2(internalToDst_ratio / cacheSettings_->maxZoom)) + 0.1;  // Force round up.
    
    // If this is the case, we can just render ourselves.
    if (minRenderCallDepth < 0) {
        std::cout << "Rendering self." << std::endl;

        return 1;
    }

    return static_cast<size_t>(minRenderCallDepth);
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
    cacheState_->cacheSize += internalWidth_ * internalHeight_;

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
    assert(children_[TOP_LEFT] != nullptr || children_[TOP_LEFT] == children_[TOP_RIGHT] &&
                                                     children_[BOT_LEFT] == children_[BOT_RIGHT] &&
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

size_t QuadTreeCache::Node::getSize() const {
    if (!hasChildren()) {
        if (rendered_ != nullptr) {
            return internalWidth_ * internalHeight_;
        }

        return 0;
    }

    size_t result = 0;
    for (size_t i = 0; i < children_.size(); i++) { result += children_[i]->getSize(); }
    return result;
}
