/**
 * Xournal++
 *
 * @brief Small class for a square with a padding
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "model/Point.h"
#include "util/Rectangle.h"

class PaddedBox final {
public:
    PaddedBox() = default;
    PaddedBox(const Point& c, double halfSize, double softPadding, double hardPaddingCoeff = 1.0):
            center(c),
            halfSize(halfSize),
            halfSizeWithSoftPadding(halfSize + softPadding),
            halfSizeWithHardPadding(halfSize + hardPaddingCoeff * softPadding) {}

private:
    Point center;
    double halfSize;
    /// @brief The padding used for repainting/rerendering purposes. Typically larger than halfSizeWithHardPadding
    double halfSizeWithSoftPadding;
    /// @brief The padding used for intersecting the padded box with anything else
    double halfSizeWithHardPadding;

public:
    xoj::util::Rectangle<double> getInnerRectangle() const {
        return {center.x - halfSize, center.y - halfSize, 2 * halfSize, 2 * halfSize};
    }
    xoj::util::Rectangle<double> getOuterRectangle() const {
        return {center.x - halfSizeWithHardPadding, center.y - halfSizeWithHardPadding, 2 * halfSizeWithHardPadding,
                2 * halfSizeWithHardPadding};
    }

    void addToRange(Range& range) const {
        range.addPoint(center.x - halfSizeWithSoftPadding, center.y - halfSizeWithSoftPadding);
        range.addPoint(center.x + halfSizeWithSoftPadding, center.y + halfSizeWithSoftPadding);
    }
};
