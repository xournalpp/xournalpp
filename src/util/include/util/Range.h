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

class Range final {
public:
    Range();
    Range(double x, double y);

    void addPoint(double x, double y);

    double getX() const;
    double getY() const;
    double getWidth() const;
    double getHeight() const;

    double getX2() const;
    double getY2() const;

private:
    double x1{};
    double x2{};
    double y1{};
    double y2{};
};
