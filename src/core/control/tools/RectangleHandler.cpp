#include "RectangleHandler.h"

#include <algorithm>  // for max

#include "control/Control.h"                       // for Control
#include "control/settings/Settings.h"             // for Settings
#include "control/tools/BaseShapeHandler.h"        // for BaseShapeHandler
#include "control/tools/SnapToGridInputHandler.h"  // for SnapToGridInputHan...
#include "model/Point.h"                           // for Point


RectangleHandler::RectangleHandler(Control* control, const PageRef& page, bool flipShift, bool flipControl):
        BaseShapeHandler(control, page, flipShift, flipControl) {}

RectangleHandler::~RectangleHandler() = default;

auto RectangleHandler::createShape(bool isAltDown, bool isShiftDown, bool isControlDown)
        -> std::pair<std::vector<Point>, Range> {
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

    Point p1;
    if (!this->modControl) {
        p1 = this->startPoint;
    } else {
        // Control is down - drawing from center
        p1 = Point(this->startPoint.x - width, this->startPoint.y - height);
    }

    Point p2 = Point(this->startPoint.x + width, this->startPoint.y + height);

    Range rg(p1.x, p1.y);
    rg.addPoint(p2.x, p2.y);

    return {{p1, {p1.x, p2.y}, p2, {p2.x, p1.y}, p1}, rg};
}
