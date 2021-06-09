/*
 * Xournal++
 *
 * Handles input to draw polynomial functions (x²,x³,x⁴,x⁵)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "BaseStrokeHandler.h"

class PolyHandler: public BaseStrokeHandler {
public:
    PolyHandler(XournalView* xournal, XojPageView* redrawable, const PageRef& page, bool flipShift = false,
                   bool flipControl = false);
    virtual ~PolyHandler();

private:
    virtual void drawShape(Point& currentPoint, const PositionInputData& pos);

private:
    Point startPoint;
    bool started = false;
};
