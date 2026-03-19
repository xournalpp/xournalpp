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

#include "util/Point.h"

namespace xoj::util {
template <typename T>
struct Point;
template <typename T>
class Rectangle;
};  // namespace xoj::util

class Range;
class XojPageView;

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

    QuadCache() = delete;
    QuadCache(const Range& area, int DPIscaling, XojPageView* view);
    QuadCache(QuadCache&&) = delete;
    QuadCache(const QuadCache&) = delete;
    QuadCache& operator=(QuadCache&&) = delete;
    QuadCache& operator=(const QuadCache&) = delete;
    ~QuadCache();

    /// Paint the content of the surfaces to the target cairo context - calls for asynchronous rendering if needed
    void paintTo(cairo_t* targetCr) const;
    /// Call an asynchronous rendering of missing nodes to cover the given range with the given zoom level
    bool preload(const Range& rg, double zoom);

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
    void assignBufferToNode(std::unique_ptr<Mask> mask, unsigned depth, const Range& area);

    /// Create a mask for the designated location
    std::unique_ptr<Mask> makeSuitableMask(unsigned depth, const Range& area) const;

private:
    XojPageView* view;

    int DPIscaling;
    double zoom;  ///< Zoom level of the crudest rendering (= the root, whose depth is 0)
    class Node;
    std::unique_ptr<Node> root;  ///< NEVER nullptr
};
};  // namespace xoj::view
