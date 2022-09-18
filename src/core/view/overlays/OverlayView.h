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

class OverlayBase;

namespace xoj::view {
class Repaintable;
class OverlayView {
public:
    OverlayView(const Repaintable* parent): parent(parent) {}
    virtual ~OverlayView() = default;

    /**
     * @brief Draws the overlay to the given context
     */
    virtual void draw(cairo_t* cr) const = 0;

    virtual bool isViewOf(const OverlayBase* overlay) const = 0;

protected:
    const Repaintable* parent;
};
};  // namespace xoj::view
