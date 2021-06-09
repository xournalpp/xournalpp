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

#include <cmath>

#include "BaseStrokeHandler.h"

class EllipseHandler: public BaseStrokeHandler {
public:
    EllipseHandler(XournalView* xournal, XojPageView* redrawable, const PageRef& page, bool flipShift = false,
                   bool flipControl = false);
    virtual ~EllipseHandler();

private:
    void createPath(const Point& p, bool snapToGrid) override;
    const Path& getPath() const override;
    virtual void drawShape(Point& currentPoint, const PositionInputData& pos) override;

protected:
    std::shared_ptr<Spline> path;
};
