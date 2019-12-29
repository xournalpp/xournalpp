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

#include <string>
#include <vector>

#include "model/Point.h"

#include "XournalType.h"

class EraseableStrokePart {
public:
    EraseableStrokePart(Point a, Point b);
    EraseableStrokePart(double width);
    virtual ~EraseableStrokePart();

private:
    EraseableStrokePart(const EraseableStrokePart& part);

public:
    void addPoint(Point p);
    double getWidth() const;

    GList* getPoints();

    void clearSplitData();
    void splitFor(double halfEraserSize);

    EraseableStrokePart* clone();

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

    GList* points = nullptr;

    double x = 0;
    double y = 0;
    double elementWidth = 0;
    double elementHeight = 0;

    friend class EraseableStroke;
};
