#include "EllipseHandler.h"

#include <algorithm>  // for max
#include <cassert>
#include <cmath>  // for abs, pow, M_PI, cos

#include "control/Control.h"                       // for Control
#include "control/settings/Settings.h"             // for Settings
#include "control/tools/BaseShapeHandler.h"        // for BaseShapeHandler
#include "control/tools/SnapToGridInputHandler.h"  // for SnapToGridInputHan...
#include "model/Point.h"                           // for Point

EllipseHandler::EllipseHandler(Control* control, const PageRef& page, bool flipShift, bool flipControl):
        BaseShapeHandler(control, page, flipShift, flipControl) {}

EllipseHandler::~EllipseHandler() = default;

auto EllipseHandler::createShape(bool isAltDown, bool isShiftDown, bool isControlDown)
        -> std::pair<std::vector<Point>, Range> {
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
    double center_x = 0;
    double center_y = 0;

    if (!this->modControl) {
        radiusX = 0.5 * width;
        radiusY = 0.5 * height;
        center_x = this->startPoint.x + radiusX;
        center_y = this->startPoint.y + radiusY;
    } else {
        // control key down, draw centered at cursor
        radiusX = width;
        radiusY = height;
        center_x = this->startPoint.x;
        center_y = this->startPoint.y;
    }

    /*
     * Set resolution depending on the radius (heuristic)
     */
    auto nbPtsPerQuadrant =
            static_cast<unsigned int>(std::ceil(5 + 0.3 * (std::abs(radiusX) + std::abs(radiusY))));
    const double stepAngle = M_PI_2 / nbPtsPerQuadrant;

    std::pair<std::vector<Point>, Range> res;
    std::vector<Point>& shape = res.first;

    /*
     * This call to reserve() makes the calls to std::transform() below safe.
     * DO NOT REMOVE
     * NB: the +1 is necessary to add a copy of the first point and close the ellipse.
     */
    shape.reserve(4 * nbPtsPerQuadrant + 1);

    shape.emplace_back(center_x + radiusX, center_y);
    for (unsigned int j = 1U; j < nbPtsPerQuadrant; j++) {
        const double tgtAngle = stepAngle * j;
        const double centerAngle = 0.25 * (std::atan2(std::abs(radiusY) * std::sin(tgtAngle), std::abs(radiusX) * std::cos(tgtAngle))) + 0.75 * tgtAngle;
        double xp = center_x + radiusX * std::cos(centerAngle);
        double yp = center_y + radiusY * std::sin(centerAngle);
        shape.emplace_back(xp, yp);
    }
    shape.emplace_back(center_x, center_y + radiusY);

    // The following std::transform() are only safe because no reallocations will happen (see reserve() above).
    // Symmetry for second quadrant
    assert(shape.capacity() >= 2 * shape.size() - 1);
    std::transform(std::next(shape.rbegin()), shape.rend(), std::back_inserter(shape),
                   [&](const Point& p) { return Point(2 * center_x - p.x, p.y); });

    // Symmetry for the second half
    assert(shape.capacity() >= 2 * shape.size() - 1);
    std::transform(std::next(shape.rbegin()), shape.rend(), std::back_inserter(shape),
                   [&](const Point& p) { return Point(p.x, 2 * center_y - p.y); });

    Range rg(center_x + radiusX, center_y + radiusY);
    rg.addPoint(center_x - radiusX, center_y - radiusY);
    res.second = rg;

    return res;
}
