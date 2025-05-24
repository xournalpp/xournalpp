#include "EllipseHandler.h"

#include <algorithm>  // for max
#include <cmath>      // for abs, pow, M_PI, cos

#include "control/Control.h"                       // for Control
#include "control/settings/Settings.h"             // for Settings
#include "control/tools/BaseShapeHandler.h"        // for BaseShapeHandler
#include "control/tools/SnapToGridInputHandler.h"  // for SnapToGridInputHan...
#include "model/Point.h"                           // for Point
#include "model/path/Spline.h"

EllipseHandler::EllipseHandler(Control* control, const PageRef& page, bool flipShift, bool flipControl):
        BaseShapeHandler(control, page, flipShift, flipControl) {}

EllipseHandler::~EllipseHandler() = default;

auto EllipseHandler::createShape(bool isAltDown, bool isShiftDown, bool isControlDown)
        -> std::pair<std::shared_ptr<Path>, Range> {
    /**
     * Snap point to grid (if enabled - Alt key pressed will toggle)
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
        // make circle
        width = (this->modControl) ? std::hypot(width, height) :
                                     std::copysign(std::max(std::abs(width), std::abs(height)), width);
        height = std::copysign(width, height);
    }

    double radiusX = 0;
    double radiusY = 0;
    double centerX = 0;
    double centerY = 0;

    if (!this->modControl) {
        radiusX = 0.5 * width;
        radiusY = 0.5 * height;
        centerX = this->startPoint.x + radiusX;
        centerY = this->startPoint.y + radiusY;
    } else {
        // control key down, draw centered at cursor
        radiusX = width;
        radiusY = height;
        centerX = this->startPoint.x;
        centerY = this->startPoint.y;
    }
    radiusX = std::abs(radiusX);  //  For bounding box computation
    radiusY = std::abs(radiusY);  //

    return std::make_pair(std::make_shared<Spline>(Spline::MAKE_ELLIPSE, Point(centerX, centerY), radiusX, radiusY),
                          Range(centerX - radiusX, centerY - radiusY, centerX + radiusX, centerY + radiusY));
}
