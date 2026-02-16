/*
 * Xournal++
 *
 * A quad-tree whose nodes contain image buffers
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <chrono>
#include <functional>
#include <memory>
#include <vector>

#include <cairo.h>

#include "util/Range.h"

namespace xoj::util {
template <typename T>
struct Point;
};  // namespace xoj::util

namespace xoj::view {
class Mask;

/**
 * @brief QuadCache class: a quadratic tree corresponding to a drawn area (typically a page).
 * Each leaf contains a pixel buffer representing part of the area.
 */
class QuadCache {
public:
    static constexpr int TILE_SIZE = 2048 * 2048;  ///< Each tile has (about) TILE_SIZE pixels
    static constexpr unsigned TILE_SIZE_IN_MB = xoj::view::QuadCache::TILE_SIZE * 4 / 1024 / 1024;  // 4 bytes per pixel

    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;
    class Node;
    using Depth = unsigned int;

    struct TileInfo {
        Range area;
        Depth depth;
    };

    QuadCache() = delete;
    QuadCache(QuadCache&&) = delete;
    QuadCache(const QuadCache&) = delete;
    QuadCache& operator=(QuadCache&&) = delete;
    QuadCache& operator=(const QuadCache&) = delete;

    /**
     * Constructs a cache covering the given area
     * @param renderNode A function called when rendering a node is required. The function could either:
     *       * Call an asynchronous rendering - use QuadCache::assignBufferToNode() to give the buffer to the right Node
     *         once it is ready
     *       * Render synchronously and use Node::assignBuffer() to give the buffer to the Node directly
     *      It must return true if the render was synchronous (so if the node now has a valid and up-to-date buffer)
     */
    QuadCache(const Range& area, int DPIscaling, std::function<bool(Node&)> renderNode);
    ~QuadCache();

    /// Paint the content of the surfaces to the target cairo context - calls for rendering if needed
    void paintTo(cairo_t* targetCr);
    /// Call a rendering of missing nodes to cover the given range with the given zoom level
    void preload(const Range& rg, double zoom);
    /**
     * Paint the content of a single tile - creates the corresponding Node if needed and orders a rendering of the Node
     * @return false if the Node did not exist and the rendering is asynchronous. In this case, nothing is painted to
     *         the cairo context.
     */
    bool paintSingleTile(cairo_t* cr, const TileInfo& ti);
    /**
     * Paint the given context and preloads around the context's clip rectangle by adding a padding proportional to
     * preloadPaddingCoefficient and to the size of the clip rectangle
     */
    void paintAndPreload(cairo_t* cr, double preloadPaddingCoefficient);

    /// Get all the buffers which intersect rg. The cairo_t's instances are owned by the QuadCache
    std::vector<cairo_t*> getSurfacesFor(const Range& rg);

    /// Mark all the content as outdated. This will trigger an asynchronous rerendering the next time a node is needed
    void markAsOutdated();

    /// Changes the area covered by the cache. Deletes all existing buffers.
    void changeArea(const Range& area);

    void clear();

    inline int getDPIscaling() const { return DPIscaling; }

    /// Get a list of the last-used dates of all the buffers
    std::vector<TimePoint> getBuffersLastUsedDates() const;
    /// Remove all node whose buffer has not been used since the time point - Returns true if the Cache is now empty
    bool prune(TimePoint date);

    /// Find the node (if any) corresponding to the given location and gives it the buffer
    void assignBufferToNode(std::unique_ptr<Mask> mask, const TileInfo& ti);

    /// Create a mask for the designated location
    std::unique_ptr<Mask> makeSuitableMask(const TileInfo& ti) const;


    /**
     * Prunes a group of QuadCaches so the total used memory doesn't exceed maxMemorySize (in MB)
     *
     * A group of QuadCache is a vector of dereferencable (possibly smart) pointers. For each one of those pointers p,
     * if the pointed-to object's cache no longer holds any buffer, the object itself is "pruned" by the assignment
     *   p = nullptr;
     * It must be safe and not lead to a memory leak (either p is a smart pointer or the vector does not own the data).
     */
    template <typename PointerToCacheParent>
    static void pruneGroup(std::vector<PointerToCacheParent>& caches, unsigned int maxMemorySize) {
        std::vector<xoj::view::QuadCache::TimePoint> dates;

        for (auto& p: caches) {
            auto d = p->getCache()->getBuffersLastUsedDates();
            if (!d.empty()) {
                dates.reserve(dates.size() + d.size());
                std::copy(d.begin(), d.end(), std::back_inserter(dates));
            }
        }

        auto maxNbNodes = maxMemorySize / TILE_SIZE_IN_MB;

        if (dates.size() > maxNbNodes) {
            std::sort(dates.begin(), dates.end());
            auto ref = dates[dates.size() - maxNbNodes];
            for (auto& p: caches) {
                if (p->getCache()->prune(ref)) {
                    p = nullptr;  // This entry no longer has a buffer
                }
            }
            std::erase(caches, nullptr);
        }
    }

    class Node final {
    public:
        Node(Range area, Depth lvl, const QuadCache& parent);
        ~Node();

        enum Child : unsigned char { TOP_LEFT, BOTTOM_LEFT, TOP_RIGHT, BOTTOM_RIGHT };

        /// Assigns the given buffer to the node. Assumes the buffer is right for the node (right area and zoom level)
        void assignBuffer(std::unique_ptr<Mask> buffer);

        inline const Range& getArea() const { return area; }
        inline Depth getDepth() const { return depth; }

    private:
        /// Makes the buffer either synchronously (returns true) or asynchronously (returns false)
        bool renderBuffer();

        /**
         * tgtDepth = minimum level to ensure a good enough resolution
         * Assumes paintRg is contained in preloadRg
         */
        void paintAndPreload(cairo_t* cr, const Range& paintRg, const Range& preloadRg, Depth tgtDepth,
                             const Node* nearestAncestorWithBuffer);

        void preload(const Range& rg, Depth level);

        /**
         * Removes any data that hasn't been used since ref
         * Returns true if this node can be destroyed
         */
        bool prune(const TimePoint& ref);

        void appendBuffersLastUsedDates(std::vector<TimePoint>& res) const;

        void clear();

        /// Appends to res the existing buffers that intersect the given Range
        void appendSurfacesForRange(const Range& rg, std::vector<cairo_t*>& res) const;

        /// Returns, if it exists, the node with prescribed depth and containing the point
        Node* getNodeAt(Depth depth, const xoj::util::Point<double>& p);
        /// Returns the node with prescribed depth and containing the point - Creates it (and its ancestors) if needed
        Node* ensureNodeExists(Depth depth, const xoj::util::Point<double>& p);

        void markAsOutdated();

        /// Set a new area and discard all children and buffers (only use on the root)
        void changeArea(const Range& rg);

        friend QuadCache;

    private:
        std::array<std::unique_ptr<Node>, 4> children;
        std::unique_ptr<Mask> buffer;

        bool outdated;  ///< The buffer is outdated and needs rerendering
        Depth depth;    ///< Depth of the node - the root has depth 0
        Range area;     ///< The user-space area covered by this node

        const QuadCache& parent;

        /// Last time this node's buffer was used
        mutable TimePoint lastNeeded;
    };

private:
    /**
     * Function called when rendering a node is required. The function could either:
     *  * Call an asynchronous rendering - use QuadCache::assignBufferToNode() to give the buffer to the right Node
     *  * Render synchronously and use Node::assignBuffer() to give the buffer to the Node directly
     * @return true if the render was synchronous (so if the node now has a valid and up-to-date buffer)
     */
    std::function<bool(Node&)> renderNode;

    int DPIscaling;
    double zoom;  ///< Zoom level of the crudest rendering (= the root, whose depth is 0)

    Node root;
};
};  // namespace xoj::view
