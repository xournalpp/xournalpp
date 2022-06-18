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
    void calc(const Point* pt, int start, int end);

private:
    double mass{};
    double sx{};
    double sy{};
    double sxx{};
    double sxy{};
    double syy{};
};
