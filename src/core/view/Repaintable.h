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

namespace xoj::view {

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

    virtual int getDPIScaling() const = 0;

    // Get the current zoom.
    virtual double getZoom() const = 0;

    // Width and height, in local coordinates
    virtual double getWidth() const = 0;
    virtual double getHeight() const = 0;

    /**
     * @brief Flag a region as dirty. Dirty regions will get redrawn at the next screen refresh.
     */
    virtual void flagDirtyRegion(const Range& rg) const = 0;
};
};  // namespace xoj::view
