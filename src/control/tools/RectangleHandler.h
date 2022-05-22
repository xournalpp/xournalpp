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
    virtual ~RectangleHandler();

private:
    std::vector<Point> createShape(const PositionInputData& pos) override;

private:
};
