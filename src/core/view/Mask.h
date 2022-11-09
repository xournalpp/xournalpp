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

#include "util/raii/CairoWrappers.h"

class Range;

namespace xoj::view {

/**
 * @brief Mask class: a cairo surface and its context to draw on, with the later purpose of blitting or using as a
 * buffer.
 */
class Mask {
public:
    Mask() = default;
    /**
     * @brief Create a mask tailored for the specified target
     * @param target A cairo surface similar to that on which the mask will be used
     * @param extent The extent of the mask, in local coordinates (i.e. in the coordinates of the cairo context(s) on
     * which the mask will be used).
     * @param zoom The local zoom ratio (zoom ratio of the cairo context(s) on which the mask will be used).
     * @param DPIScaling The targeted screen DPI scaling
     * @param contentType The intended content of the mask
     */
    Mask(cairo_surface_t* target, const Range& extent, double zoom, int DPIScaling,
         cairo_content_t contentType = CAIRO_CONTENT_ALPHA);
    cairo_t* get();
    bool isInitialized() const;
    /**
     * @brief Use the surface as a mask
     */
    void blitTo(cairo_t* targetCr) const;
    /**
     * @brief Paint the content of the surface to the target cairo context
     */
    void paintTo(cairo_t* targetCr) const;
    /**
     * @brief Erase all the surface's content
     */
    void wipe();

    /**
     * @brief Delete the mask
     */
    void reset();

private:
    xoj::util::CairoSPtr cr;
};
};  // namespace xoj::view
