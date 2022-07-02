/*
 * Xournal++
 *
 * Range
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <limits>

namespace xoj::util {
template <typename Float>
class Rectangle;
};

class Range final {
public:
    Range() = default;  // Empty range
    Range(double x, double y): minX(x), minY(y), maxX(x), maxY(y) {}
    Range(double minX, double minY, double maxX, double maxY): minX(minX), minY(minY), maxX(maxX), maxY(maxY) {}
    explicit Range(const xoj::util::Rectangle<double>& r);

    void addPoint(double x, double y);

    [[nodiscard]] Range unite(const Range& other) const;
    [[nodiscard]] Range intersect(const Range& other) const;

    [[nodiscard]] double getX() const;
    [[nodiscard]] double getY() const;
    [[nodiscard]] double getWidth() const;
    [[nodiscard]] double getHeight() const;

    void addPadding(double padding);

    [[nodiscard]] bool empty() const;
    [[nodiscard]] bool isValid() const;

    double minX = std::numeric_limits<double>::max();
    double minY = std::numeric_limits<double>::max();
    double maxX = std::numeric_limits<double>::lowest();
    double maxY = std::numeric_limits<double>::lowest();
};
