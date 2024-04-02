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

namespace xoj::util {

template <typename T>
struct Point {

    // Todo (c++20): remove constructors
    constexpr Point() = default;
    constexpr Point(T x, T y): x(x), y(y) {}

    [[maybe_unused]] constexpr auto distance(Point p) const -> double {
        return std::hypot(p.x - this->x, p.y - this->y);
    }

    [[maybe_unused]] constexpr auto operator-(Point p) const -> Point { return {this->x - p.x, this->y - p.y}; }
    [[maybe_unused]] constexpr auto operator-=(Point p) -> Point& { return *this = *this - p; }

    [[maybe_unused]] constexpr auto operator+(Point p) const -> Point { return {this->x + p.x, this->y + p.y}; }
    [[maybe_unused]] constexpr auto operator+=(Point p) -> Point& { return *this = *this + p; }

    [[maybe_unused]] constexpr auto operator/(T k) const -> Point { return {this->x / k, this->y / k}; }
    [[maybe_unused]] constexpr auto operator/=(T k) -> Point& { return *this = *this / k; }

    [[maybe_unused]] constexpr auto operator*(T k) const -> Point { return {this->x * k, this->y * k}; }
    [[maybe_unused]] constexpr auto operator*=(T k) -> Point& { return *this = *this * k; }

    [[maybe_unused]] constexpr friend auto operator==(Point const& lhs, Point const& rhs) -> bool {
        return std::tie(lhs.x, lhs.y) == std::tie(rhs.x, rhs.y);
    }

    [[maybe_unused]] constexpr friend auto operator!=(Point const& lhs, Point const& rhs) -> bool {
        return !(lhs == rhs);
    }

    T x{};
    T y{};
};
}  // namespace xoj::util
