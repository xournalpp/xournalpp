/*
 * Xournal++
 *
 * Handles input to draw gaussian curve
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "BaseStrokeHandler.h"

class GaussHandler: public BaseStrokeHandler {
public:
    GaussHandler(XournalView* xournal, XojPageView* redrawable, const PageRef& page, bool flipShift = false,
                   bool flipControl = false);
    virtual ~GaussHandler();

private:
    virtual void drawShape(Point& currentPoint, const PositionInputData& pos);

private:
    Point startPoint;
    bool started = false;
};
