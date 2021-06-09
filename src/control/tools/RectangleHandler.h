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

#include "PLShapeHandler.h"

class RectangleHandler: public PLShapeHandler {
public:
    RectangleHandler(XournalView* xournal, XojPageView* redrawable, const PageRef& page, bool flipShift = false,
                     bool flipControl = false);
    virtual ~RectangleHandler();

private:
    virtual void drawShape(Point& currentPoint, const PositionInputData& pos) override;
};
