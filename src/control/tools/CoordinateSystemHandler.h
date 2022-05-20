/*
 * Xournal++
 *
 * Handles input to draw an coordinate system
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "BaseStrokeHandler.h"

class CoordinateSystemHandler: public BaseStrokeHandler {
public:
    CoordinateSystemHandler(XournalView* xournal, XojPageView* redrawable, const PageRef& page, bool flipShift = false,
                            bool flipControl = false);
    virtual ~CoordinateSystemHandler();

private:
    void drawShape(Point& currentPoint, const PositionInputData& pos,
                   const std::lock_guard<std::recursive_mutex>&) override;

private:
    Point startPoint;
    bool started = false;
};
