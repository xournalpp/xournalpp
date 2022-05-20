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
    void drawShape(Point& currentPoint, const PositionInputData& pos,
                   const std::lock_guard<std::recursive_mutex>& lock) override;

private:
    Point startPoint;
    bool started = false;
};
