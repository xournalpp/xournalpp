/*
 * Xournal++
 *
 * Handles input to draw an coordinate system
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "model/PageRef.h"  // for PageRef
#include "model/Point.h"    // for Point

#include "BaseShapeHandler.h"  // for BaseShapeHandler

class PositionInputData;
class XojPageView;
class XournalView;

class CoordinateSystemHandler: public BaseShapeHandler {
public:
    CoordinateSystemHandler(XournalView* xournal, XojPageView* redrawable, const PageRef& page, bool flipShift = false,
                            bool flipControl = false);
    ~CoordinateSystemHandler() override;

private:
    void drawShape(Point& currentPoint, const PositionInputData& pos) override;

private:
    Point startPoint;
    bool started = false;
};
