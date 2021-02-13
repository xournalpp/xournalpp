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

#include "PLShapeHandler.h"

class CoordinateSystemHandler: public PLShapeHandler {
public:
    CoordinateSystemHandler(XournalView* xournal, XojPageView* redrawable, const PageRef& page, bool flipShift = false,
                            bool flipControl = false);
    virtual ~CoordinateSystemHandler();

private:
    virtual void drawShape(Point& currentPoint, const PositionInputData& pos);
};
