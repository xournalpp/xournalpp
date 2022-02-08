/*
 * Xournal++
 *
 * Handles input to draw a rectangle
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "BaseStrokeHandler.h"

class RectangleHandler: public BaseStrokeHandler {
public:
    RectangleHandler(XournalView* xournal, XojPageView* redrawable, const PageRef& page, bool flipShift = false,
                     bool flipControl = false);
    ~RectangleHandler() override;

private:
    void drawShape(Point& currentPoint, const PositionInputData& pos) override;

private:
    Point startPoint;
    bool started = false;
};
