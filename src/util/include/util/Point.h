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

#include <cmath>
#include <tuple>

namespace utl {

template <typename T>
struct Point {

    Point() = default;
    Point(T x, T y): x(x), y(y) {}

    [[maybe_unused]] double distance(Point p) { return std::hypot(p.x - this->x, p.y - this->y); }

    [[maybe_unused]] Point operator-(Point p) const { return {this->x - p.x, this->y - p.y}; }
    [[maybe_unused]] Point& operator-=(Point p) { return *this = *this - p; }

    [[maybe_unused]] Point operator+(Point p) const { return {this->x + p.x, this->y + p.y}; }
    [[maybe_unused]] Point& operator+=(Point p) { return *this = *this + p; }

    [[maybe_unused]] Point operator/(T k) const { return {this->x / k, this->y / k}; }
    [[maybe_unused]] Point& operator/=(T k) { return *this = *this / k; }

    [[maybe_unused]] Point operator*(T k) const { return {this->x * k, this->y * k}; }
    [[maybe_unused]] Point& operator*=(T k) { return *this = *this * k; }

    [[maybe_unused]] friend bool operator==(Point const& lhs, Point const& rhs) {
        return std::tie(lhs.x, lhs.y) == std::tie(rhs.x, rhs.y);
    }

    [[maybe_unused]] friend bool operator!=(Point const& lhs, Point const& rhs) { return !(lhs == rhs); }

    T x{};
    T y{};
};
}  // namespace utl
