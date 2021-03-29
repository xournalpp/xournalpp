/*
 * Xournal++
 *
 * A quad-tree cache.
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <functional>
#include <memory>

#include <cairo.h>

#include "Rectangle.h"

class QuadTreeCache {
public:
    /**
     * Specifies the order in which regions are removed from the cache
     * (or rendered quality of a region lowered).
     *
     * VIEWPORT_THEN_LRU causes the cache to assume that the most recently rendered
     *  region predicts the zoom and viewport size of future renders; it assumes that
     *  we can safely reduce the quality of regions with >= the most recent zoom.
     *
     * LRU doesn't take the viewport into account and removes least recently used regions
     *  from the cache.
     */
    enum class DecachePolicy { VIEWPORT_THEN_LRU, LRU };

    struct CacheParams {
        /**
         * The size (in rendered pixels) of each node in the tree.
         * This should be a square number.
         */
        size_t entrySize{512 * 512};

        /**
         * The maximum amount a region can be scaled before it needs to be
         * re-cached with a greater size.
         */
        double maxZoom{1.0};

        /**
         * The maximum size (in rendered pixels) of the cache.
         * If the cache gets lager than this, it starts removing
         * regions from the cache.
         */
        size_t maxSize{static_cast<size_t>(-1)};

        /**
         * See {@link QuadTreeCache::DecachePolicy}
         */
        DecachePolicy uncachePolicy{DecachePolicy::VIEWPORT_THEN_LRU};
    };

public:
    using Rect = Rectangle<double>;
    using RenderFn = std::function<void(cairo_t*, const Rect&)>;

    /**
     * @param renderFn Renders a given rectangle to a given cairo context.
     *  This function is used to populate the cache with data. Call damage(Rect)
     *  to force a region's re-render.
     * @param pageRect Gives the region of pixels managed by this in cache/renderFn-coordinates.
     * @param cacheSettings Specifies cache behavior.
     */
    QuadTreeCache(RenderFn renderFn, const Rect& pageRect, const CacheParams& cacheSettings);
    ~QuadTreeCache();

    /**
     * @brief Invalidate/mark a region to be re-rendered. This might be done if
     *   (for example) this cache's source changes in that region.
     * @param region A region that must be re-rendered before drawing again.
     */
    void damage(const Rect& region);

    /**
     * @brief Render a region from the cache and the cache's source.
     * @param cr The target, to which the cache is rendered (at coordinates specified by dstRegion).
     * @param srcRegion The region to fetch from the cache (or fetch from this cache's source, or fetch
     *   from this cache's source and store in this cache).
     * @param dstRegion The region to render to the context. The cache will ensure that what is rendered
     *
     *
     * For example,
     *   cache.render(some_context, Rect { 1.0, 1.0, 2.0, 2.0 }, Rect { 0.0, 0.0, 1.0, 1.0 });
     *  renders a rectangle stored in the cache at (1, 1) with width and height 2 to a region at (0, 0)
     *  on some_context.
     *  It may call renderFn, or render from the cache (if the region is available).
     *
     */
    void render(cairo_t* cr, const Rect& srcRegion, const Rect& dstRegion);

    /**
     * @brief Clears cached data.
     */
    void clear();

private:
    CacheParams cacheSettings_;

    class Node;
    std::unique_ptr<Node> root_;
};
