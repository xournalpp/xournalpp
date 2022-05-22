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
    virtual ~EllipseHandler();

private:
    std::vector<Point> createShape(const PositionInputData& pos) override;

private:
};
