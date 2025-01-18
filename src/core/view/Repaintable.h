/*
 * Xournal++
 *
 *
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

class Range;
class ZoomControl;

namespace xoj::util {
template <typename T>
class Rectangle;
};

namespace xoj::view {
class OverlayView;
class ToolView;

class Repaintable {
public:
    Repaintable() = default;
    virtual ~Repaintable() = default;

    Repaintable(const Repaintable&) = delete;
    Repaintable(Repaintable&&) = delete;

    /**
     * @brief Get the Repaintable's visible part, in local coordinates
     * @return A range. It could be empty.
     */
    virtual Range getVisiblePart() const = 0;

    // Get the current zoom.
    virtual double getZoom() const = 0;
    virtual ZoomControl* getZoomControl() const = 0;

    // Width and height, in local coordinates
    virtual double getWidth() const = 0;
    virtual double getHeight() const = 0;

    // Convert a rectangle from local coordinates to global coordinates
    virtual xoj::util::Rectangle<double> toWindowCoordinates(const xoj::util::Rectangle<double>& r) const = 0;

    /**
     * @brief Flag a region as dirty. Dirty regions will get redrawn at the next screen refresh.
     */
    virtual void flagDirtyRegion(const Range& rg) const = 0;

    /**
     * @brief Draw the ToolView to the repaintable buffer (if any), remove a tool view and repaint the given range
     *      Called just before the tool sequence ends and the handler is deleted.
     */
    virtual void drawAndDeleteToolView(ToolView* v, const Range& rg) = 0;

    /**
     * @brief Remove a overlay view and repaint the given range
     *      The given range is supposed to be big enough so that repainting this area will indeed erase the overlay from
     * the display
     */
    virtual void deleteOverlayView(OverlayView* v, const Range& rg) = 0;
};
};  // namespace xoj::view
