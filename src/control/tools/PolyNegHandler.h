/*
 * Xournal++
 *
 * Handles input to draw sqrt(x) x^-1 x^-2 x^-3 functions
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "BaseStrokeHandler.h"

class PolyNegHandler: public BaseStrokeHandler {
public:
    PolyNegHandler(XournalView* xournal, XojPageView* redrawable, const PageRef& page, bool flipShift = false,
                   bool flipControl = false);
    virtual ~PolyNegHandler();

private:
    virtual void drawShape(Point& currentPoint, const PositionInputData& pos);

private:
    Point startPoint;
    bool started = false;
};
