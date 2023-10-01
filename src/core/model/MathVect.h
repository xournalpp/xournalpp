/*
 * Xournal++
 *
 * Rudimentary 2D/3D mathematical vectors
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

class Point;

struct MathVect2 {
public:
    MathVect2() = default;
    MathVect2(const Point& p, const Point& q);
    MathVect2(double dx, double dy);

    double dx{};
    double dy{};
    static double scalarProduct(const MathVect2 u, const MathVect2 v);
    double norm() const;
    double squaredNorm() const;
    bool isZero() const;
    MathVect2 operator+(const MathVect2& u) const;
    MathVect2 operator-(const MathVect2& u) const;
};

MathVect2 operator*(const double c, const MathVect2& u);


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
    void operator+=(const MathVect3& u);
    void operator-=(const MathVect3& u);

    double norm() const;

    /**
     * @brief Normalize the vector, assuming its original norm is initNorm
     */
    void normalize(double initNorm);

    inline void normalize() { return normalize(norm()); }

    Point translatePoint(const Point& p) const;

    static double scalarProduct(const MathVect3& u, const MathVect3& v);

    static constexpr double EPSILON = 1e-6;

public:
    double dx{};
    double dy{};
    double dz{};
};

MathVect3 operator*(const double c, const MathVect3& u);
