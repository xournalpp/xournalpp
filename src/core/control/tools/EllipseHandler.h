/*
 * Xournal++
 *
 * Handles input to draw ellipses
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "BaseStrokeHandler.h"

class EllipseHandler: public BaseStrokeHandler {
public:
    EllipseHandler(XournalView* xournal, XojPageView* redrawable, const PageRef& page, bool flipShift = false,
                   bool flipControl = false);
    ~EllipseHandler() override;

private:
    void drawShape(Point& currentPoint, const PositionInputData& pos) override;

private:
    Point startPoint;
    bool started = false;
};
