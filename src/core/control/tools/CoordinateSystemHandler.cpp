#include "CoordinateSystemHandler.h"

#include <algorithm>  // for max

#include "control/Control.h"                       // for Control
#include "control/settings/Settings.h"             // for Settings
#include "control/tools/BaseShapeHandler.h"        // for BaseShapeHandler
#include "control/tools/SnapToGridInputHandler.h"  // for SnapToGridInputHan...
#include "model/Point.h"                           // for Point
#include "model/path/PiecewiseLinearPath.h"

CoordinateSystemHandler::CoordinateSystemHandler(Control* control, const PageRef& page, bool flipShift,
                                                 bool flipControl):
        BaseShapeHandler(control, page, flipShift, flipControl) {}

CoordinateSystemHandler::~CoordinateSystemHandler() = default;

/**
 * Draw a Cartesian coordinate system.
 *
 * @param currentPoint The current point the mouse cursor is pointing to.
 * @param shiftDown Boolean to indicate if "shift" is currently pressed.
 *                  It is currently not used.
 */
auto CoordinateSystemHandler::createShape(bool isAltDown, bool isShiftDown, bool isControlDown)
        -> std::pair<std::shared_ptr<Path>, Range> {
    /**
     * Snap point to grid (if enabled)
     */
    Point c = snappingHandler.snapToGrid(this->currPoint, isAltDown);

    double width = c.x - this->startPoint.x;
    double height = c.y - this->startPoint.y;

    this->modShift = isShiftDown;
    this->modControl = isControlDown;

    Settings* settings = control->getSettings();
    if (settings->getDrawDirModsEnabled()) {
        // change modifiers based on draw dir
        this->modifyModifiersByDrawDir(width, height, true);
    }

    if (this->modShift) {
        // make square
        int signW = width > 0 ? 1 : -1;
        int signH = height > 0 ? 1 : -1;
        width = std::max(width * signW, height * signH) * signW;
        height = (width * signW) * signH;
    }

    const Point& p1 = this->startPoint;

    Range rg(p1.x, p1.y);
    rg.addPoint(p1.x + width, p1.y + height);

    if (!this->modControl) {
        // draw out from starting point
        auto shape = std::make_shared<PiecewiseLinearPath>(p1, 2);
        shape->addLineSegmentTo(p1.x, p1.y + height);
        shape->addLineSegmentTo(p1.x + width, p1.y + height);
        return {std::move(shape), rg};
    } else {
        // Control is down
        auto shape = std::make_shared<PiecewiseLinearPath>(Point(p1.x, p1.y + height), 2);
        shape->addLineSegmentTo(p1);
        shape->addLineSegmentTo(p1.x + width, p1.y);
        return {std::move(shape), rg};
    }
}
