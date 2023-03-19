/*
 * Xournal++
 *
 * A rectangle data structure over a numeric data type.
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <algorithm>
#include <optional>

#include "util/Range.h"

namespace xoj::util {  // Rectangle is already defined in windows.h

template <class T>
class Rectangle final {
public:
    Rectangle() = default;
    Rectangle(T x, T y, T width, T height): x(x), y(y), width(width), height(height) {}
    explicit Rectangle(const Range& rect):
            x(rect.getX()), y(rect.getY()), width(rect.getWidth()), height(rect.getHeight()) {}

    /**
     * Returns whether this rectangle intersects another and the intersection
     * @param other the other rectangle
     * @return whether the rectangles intersect and if so, the intersection
     */
    std::optional<Rectangle> intersects(const Rectangle& other) const {
        auto x1 = std::max(this->x, other.x);
        auto y1 = std::max(this->y, other.y);
        auto x2 = std::min(this->x + this->width, other.x + other.width);
        auto y2 = std::min(this->y + this->height, other.y + other.height);
        if (x2 > x1 && y2 > y1) {
            return {{x1, y1, x2 - x1, y2 - y1}};
        }
        return std::nullopt;
    }

    /**
     * Returns a new Rectangle with an offset specified
     * by the function arguments
     */
    Rectangle translated(T dx, T dy) const { return Rectangle(this->x + dx, this->y + dy, this->width, this->height); }

    /**
     * Computes the union of this and the other rectangle
     */
    void unite(const Rectangle& other) {
        this->width = std::max(this->x + this->width, other.x + other.width);
        this->height = std::max(this->y + this->height, other.y + other.height);
        this->x = std::min(this->x, other.x);
        this->y = std::min(this->y, other.y);
        this->width -= this->x;
        this->height -= this->y;
    }

    /**
     * Applies a scalar to this rectangle
     */
    Rectangle& operator*=(T factor) {
        x *= factor;
        y *= factor;
        width *= factor;
        height *= factor;
        return *this;
    }

    /**
     * Calculates the area
     */
    T area() const { return width * height; }

    T x{};
    T y{};
    T width{};
    T height{};
};

}  // namespace xoj::util
