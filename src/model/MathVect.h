/*
 * Xournal++
 *
 * Rudimentary 3D mathematical vectors
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "Point.h"

struct MathVect3 {
public:
    MathVect3() = default;
    /**
     * @brief Get the vector pq
     */
    MathVect3(const Point& p, const Point& q);
    MathVect3(double dx, double dy, double dz);
    
    MathVect3& operator=(const MathVect3&) = default;
    MathVect3 operator+(const MathVect3& u) const;
    MathVect3 operator-(const MathVect3& u) const;
    MathVect3 operator-() const;
    
    void operator*=(const double d);
    void operator/=(const double d);

    double norm() const;
    
    void normalize();

    Point translatePoint(const Point& p) const;
    
    static double scalarProduct(const MathVect3& u, const MathVect3& v);

    static constexpr double EPSILON = 1e-6;
    
public:
    double dx{};
    double dy{};
    double dz{};
};

MathVect3 operator*(const double c, const MathVect3& u);
