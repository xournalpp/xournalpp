/*
 * Xournal++
 *
 * Rudimentary 2D mathematical vectors
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "util/Point.h"

struct MathVect2 {
    MathVect2() = default;
    MathVect2(double dx, double dy);
    template <typename point_type, std::enable_if_t<!std::is_arithmetic_v<point_type>, bool> = true>
    MathVect2(const point_type& p, const point_type& q): dx(q.x - p.x), dy(q.y - p.y) {}

    double dx{};
    double dy{};
    static double scalarProduct(const MathVect2& u, const MathVect2& v);
    double norm() const;
    double squaredNorm() const;
    bool isZero() const;
    MathVect2 operator+(const MathVect2& u) const;
    MathVect2 operator-(const MathVect2& u) const;
    MathVect2 operator-() const;
    double argument() const;
};

MathVect2 operator*(const double c, const MathVect2& u);
template <typename point_type>
xoj::util::Point<double> operator+(const point_type& p, const MathVect2& v) {
    return {p.x + v.dx, p.y + v.dy};
}
template <typename point_type>
xoj::util::Point<double> operator-(const point_type& p, const MathVect2& v) {
    return {p.x - v.dx, p.y - v.dy};
}
