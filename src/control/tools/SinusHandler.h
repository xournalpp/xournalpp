/*
 * Xournal++
 *
 * Handles input to draw sinus functions
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "BaseStrokeHandler.h"

class SinusHandler: public BaseStrokeHandler {
public:
    SinusHandler(XournalView* xournal, XojPageView* redrawable, const PageRef& page, bool flipShift = false,
                   bool flipControl = false);
    virtual ~SinusHandler();

private:
    virtual void drawShape(Point& currentPoint, const PositionInputData& pos);

private:
    Point startPoint;
    bool started = false;
};
