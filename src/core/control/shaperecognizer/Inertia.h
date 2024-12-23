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

class Point;

class Inertia {
public:
    Inertia();
    Inertia(const Inertia& inertia);
    virtual ~Inertia();

public:
    double centerX() const;
    double centerY() const;

    double xx() const;
    double xy() const;
    double yy() const;

    double rad() const;

    double det() const;

    double getMass() const;

    void increase(Point p1, Point p2, int coef);
    void calc(const std::vector<Point>& pt, int start, int end);

private:
    double mass{};
    double sx{};    // sum of x
    double sy{};    // sum of y
    double sxx{};   // sum of x^2
    double sxy{};   // sum of x*y
    double syy{};   // sum of y^2
};
