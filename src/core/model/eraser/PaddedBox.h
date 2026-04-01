/**
 * Xournal++
 *
 * @brief Small structure for a square with a padding
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "model/Point.h"
#include "util/Rectangle.h"

struct PaddedBox {
    Point center;
    double halfSize;
    double halfSizeWithPadding;

    xoj::util::Rectangle<double> getInnerRectangle() const {
        return {center.x - halfSize, center.y - halfSize, 2 * halfSize, 2 * halfSize};
    }
    xoj::util::Rectangle<double> getOuterRectangle() const {
        return {center.x - halfSizeWithPadding, center.y - halfSizeWithPadding, 2 * halfSizeWithPadding,
                2 * halfSizeWithPadding};
    }

    void addToRange(Range& range) const {
        range.addPoint(center.x - halfSizeWithPadding, center.y - halfSizeWithPadding);
        range.addPoint(center.x + halfSizeWithPadding, center.y + halfSizeWithPadding);
    }
};
