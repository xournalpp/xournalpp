/*
 * Xournal++
 *
 * Handles input to draw an arrow
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "model/PiecewiseLinearPath.h"

#include "BaseStrokeHandler.h"

class PLShapeHandler: public BaseStrokeHandler {
public:
    PLShapeHandler(XournalView* xournal, XojPageView* redrawable, const PageRef& page, bool flipShift = false,
                   bool flipControl = false):
            BaseStrokeHandler(xournal, redrawable, page, flipShift, flipControl) {}
    virtual ~PLShapeHandler() = default;

private:
    void createPath(const Point& p, bool snapToGrid) override {
        Point snapped = snappingHandler.snapToGrid(p, snapToGrid);
        this->startPoint = snapped;
        path = std::make_shared<PiecewiseLinearPath>(snapped);
        stroke->setPath(path);
    }
    const Path& getPath() const override { return *path; }

protected:
    std::shared_ptr<PiecewiseLinearPath> path;
};
