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

#include <glib.h>

#include "model/Point.h"


class ErasableStrokePart {
public:
    ErasableStrokePart(Point a, Point b);
    ErasableStrokePart(double width);
    virtual ~ErasableStrokePart();

private:
    ErasableStrokePart(const ErasableStrokePart& part);

public:
    void addPoint(Point p);
    double getWidth() const;

    GList* getPoints();

    void clearSplitData();
    void splitFor(double halfEraserSize);

    ErasableStrokePart* clone();

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

    friend class ErasableStroke;
};
