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

#include <cairo.h>
#include <gdk/gdk.h>

#include "util/Rectangle.h"
#include "util/raii/CairoWrappers.h"

class Range;

namespace xoj::view {
/**
 * @brief Tile class: a cairo surface + its context
 * See also Mask below for a Page-coordinates-aware version
 */
class Tile final {
public:
    Tile() = default;

    /**
     * @brief Create a mask tailored for the specified target
     * @param DPIScaling The DPI scaling of the targeted use monitor
     * @param extent in user space pixels (so the real surface has more pixels if DPIScaling > 1)
     * @param contentType The intended content of the mask
     */
    Tile(int DPIScaling, const xoj::util::Rectangle<int>& extent, double zoom,
         cairo_content_t contentType = CAIRO_CONTENT_ALPHA);
    /**
     * @brief Create a mask tailored for the specified target
     * @param target A cairo surface similar to that on which the mask will be used
     * @param extent in user space pixels (so the real surface has more pixels if DPIScaling > 1)
     * @param contentType The intended content of the mask
     */
    Tile(cairo_surface_t* target, const xoj::util::Rectangle<int>& extent, double zoom,
         cairo_content_t contentType = CAIRO_CONTENT_ALPHA);

    /**
     * @brief Use the surface as a mask
     */
    void blitTo(cairo_t* targetCr) const;
    /**
     * @brief Paint the content of the surface to the target cairo context
     */
    void paintTo(cairo_t* targetCr) const;
    /**
     * @brief Paint the content of the surface to the target cairo context
     */
    void paintToWithAlpha(cairo_t* targetCr, uint8_t alpha) const;

    /**
     * @brief Delete the tile
     */
    inline void reset() { cr.reset(); }

    inline cairo_t* get() const { return cr.get(); }

    const auto& getExtent() const { return extent; }

    /**
     * Repurpose the already allocated tile to this extent and zoom
     * Assumes extent.{width,height} == this->extent.{width,height}
     */
    void repurpose(const xoj::util::Rectangle<int>& extent, double zoom);

private:
    xoj::util::CairoSPtr cr;
    xoj::util::Rectangle<int> extent;
};

/**
 * @brief Mask class: a single tile with the later purpose of blitting or using as a buffer.
 */
class Mask final {
public:
    Mask() = default;
    /**
     * @brief Create a mask tailored for the specified target
     * @param target A cairo surface similar to that on which the mask will be used
     * @param extent The extent of the mask, in local coordinates (i.e. in the coordinates of the cairo context(s) on
     * which the mask will be used: often page coordinates).
     * @param zoom The local zoom ratio (zoom ratio of the cairo context(s) on which the mask will be used).
     * @param contentType The intended content of the mask
     */
    Mask(cairo_surface_t* target, const Range& extent, double zoom, cairo_content_t contentType = CAIRO_CONTENT_ALPHA);

    /**
     * @brief Create a mask tailored for the specified target
     * @param DPIScaling The DPI scaling of the targeted use monitor
     * @param extent The extent of the mask, in local coordinates (i.e. in the coordinates of the cairo context(s) on
     * which the mask will be used: often page coordinates).
     * @param zoom The local zoom ratio (zoom ratio of the cairo context(s) on which the mask will be used).
     * @param contentType The intended content of the mask
     */
    Mask(int DPIScaling, const Range& extent, double zoom, cairo_content_t contentType = CAIRO_CONTENT_ALPHA);

    inline cairo_t* get() { return tile.get(); }
    inline bool isInitialized() const { return tile.get(); }
    /**
     * @brief Use the surface as a mask
     */
    void blitTo(cairo_t* targetCr) const;
    /**
     * @brief Paint the content of the surface to the target cairo context
     */
    void paintTo(cairo_t* targetCr) const;
    /**
     * @brief Paint the content of the surface to the target cairo context
     */
    void paintToWithAlpha(cairo_t* targetCr, uint8_t alpha) const;
    /**
     * @brief Erase all the surface's content
     */
    void wipe();

    /**
     * @brief Erase part the surface's content
     * @param rg The part to erase, in Page coordinates
     */
    void wipeRange(const Range& rg);

    /**
     * @brief Delete the mask
     */
    inline void reset() { tile.reset(); }

    inline double getZoom() const { return zoom; }

private:
    double zoom = 1.0;
    Tile tile;
};
};  // namespace xoj::view
