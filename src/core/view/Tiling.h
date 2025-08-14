/*
 * Xournal++
 *
 * Virtual class for showing overlays (e.g. active tools, selections and so on)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

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

namespace xoj::view {
class Tile;

/**
 * @brief Tiling class: a collection of tiles, with the later purpose of blitting or using as a buffer.
 */
class Tiling {
public:
    Tiling();
    Tiling(Tiling&&);
    Tiling(const Tiling&) = delete;
    Tiling& operator=(Tiling&&);
    Tiling& operator=(const Tiling&) = delete;
    ~Tiling();

    /**
     * @brief Paint the content of the surfaces to the target cairo context
     */
    void paintTo(cairo_t* targetCr) const;

    void clear();
    bool empty() const;

    inline double getZoom() const { return zoom; }
    inline void setZoom(double z) { zoom = z; }

    /// Compute what tiles are necessary to cover an area centered at c (and within rg) and creates the tiles
    void populate(int DPIscaling, const xoj::util::Point<int>& c, const Range& rg, double zoom);

    struct RetilingData {
        std::vector<xoj::util::Rectangle<int>> missingTiles;  ///< Sorted lexicographically with respect to (x,y)
        std::vector<std::unique_ptr<Tile>> unusedTiles;       ///< Allocated tiles not currently in use

        void merge(RetilingData other);
    };

    /**
     * Get the extents of missing tiles to cover the intersection of extent and an elliptic area around c
     * The result vector is sorted for the lexicographic order of the rectangle's (x,y)
     */
    RetilingData computeRetiling(const xoj::util::Point<int>& c, const Range& extent, double zoom);

    /// Create tiles. Set zoom before calling this.
    void createTiles(int DPIscaling, RetilingData retiling);

    /// Adds the tiles from other. other with then be empty.
    void append(Tiling& other);

    /// Get the tiles intersecting the given range
    std::vector<Tile*> getTilesFor(const Range& rg);

    inline const auto& getTiles() const { return tiles; }

    static size_t getEstimatedMemUsageForOneTile(int DPIscaling);

private:
    double zoom = 1.0;
    xoj::util::Point<int> center;

    std::vector<std::unique_ptr<Tile>> tiles;
};
};  // namespace xoj::view
