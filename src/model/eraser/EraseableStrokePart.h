/*
 * Xournal++
 *
 * A stroke which is temporary used if you erase a part
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <vector>

#include "model/Point.h"

class EraseableStrokePart {
public:
    EraseableStrokePart(Point a, Point b);
    EraseableStrokePart(double width);

public:
    void addPoint(Point p);
    double getWidth() const;

    std::vector<Point>& getPoints();
    std::vector<Point> const& getPoints() const;

    void clearSplitData();
    void splitFor(double halfEraserSize);

    void calcSize();

public:
    double getX() const;
    double getY() const;
    double getElementWidth() const;
    double getElementHeight() const;

    static void printDebugStrokeParts();

private:
    double width = 0;
    double splitSize = 0;

    std::vector<Point> points{};

    double x = 0;
    double y = 0;
    double elementWidth = 0;
    double elementHeight = 0;
};
