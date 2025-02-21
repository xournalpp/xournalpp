/*
 * Xournal++
 *
 * Part of the Xournal shape recognizer
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once
#include <vector>
using std::vector;
class Point;

class Inertia {
public:
    Inertia();
    Inertia(const Inertia& inertia);
    virtual ~Inertia();

public:
/* compute normalized quantities */
// Center of mass (COM) of the stroke
    double centerX() const;
    double centerY() const;

    double xx() const;
    double xy() const;
    double yy() const;

/**
    * @brief: Compute the radius of the circle if the stroke is a circle.
    * @return: The radius of the circle.
*/
    double rad() const;

    double det() const;

    double getMass() const;

    /**
    * @brief: accomulate the mass and moments of the stroke by adding a new point or removing a point.
    */
    void increase(const Point& p1, const Point& p2, int coef);
    /**
    * @brief: calculate the mass and moments of the stroke from \p start to \p end.
    */
    void calc(vector<Point>::const_iterator start, vector<Point>::const_iterator end);

private:
    double mass{};  // length of the stroke (the sum of legths between its points)
    double sx{};    // sum of weighted x
    double sy{};    // sum of weighted y
    double sxx{};   // sum of weighted x^2
    double sxy{};   // sum of weighted x*y
    double syy{};   // sum of weighted y^2
};
