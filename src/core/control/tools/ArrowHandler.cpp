#include "ArrowHandler.h"

#include <algorithm>  // for minmax_element
#include <cmath>      // for cos, sin, atan2, M_PI

#include "control/Control.h"                       // for Control
#include "control/ToolHandler.h"                   // for ToolHandler
#include "control/tools/BaseShapeHandler.h"        // for BaseShapeHandler
#include "control/tools/SnapToGridInputHandler.h"  // for SnapToGridInputHan...
#include "gui/inputdevices/PositionInputData.h"    // for PositionInputData
#include "model/Point.h"                           // for Point
#include "util/Range.h"                            // for Range

ArrowHandler::ArrowHandler(Control* control, const PageRef& page, bool doubleEnded):
        BaseShapeHandler(control, page), doubleEnded(doubleEnded) {}

ArrowHandler::~ArrowHandler() = default;

auto ArrowHandler::createShape(bool isAltDown, bool isShiftDown, bool isControlDown)
        -> std::pair<std::vector<Point>, Range> {
    Point c = snappingHandler.snap(this->currPoint, this->startPoint, isAltDown);
    const double lineLength = std::hypot(c.x - this->startPoint.x, c.y - this->startPoint.y);
    const double thickness = control->getToolHandler()->getThickness();
    const double slimness = lineLength / thickness;

    // We've now computed the line points for the arrow
    // so we just have to build the head:
    // arrowDist is the distance between the line's and the arrow's tips
    // delta is the angle between each arrow leg and the line

    // set up the size of the arrow head to be 7x the thickness of the line (regime 1)
    const double THICK1 = 7, LENGTH2 = 0.4;
    double arrowDist = thickness * THICK1;
    // but not too large compared to the line length (regime 2)
    if (slimness < THICK1 / LENGTH2) {
        arrowDist = lineLength * LENGTH2;
    }

    // an appropriate opening angle 2*delta is Pi/3 radians for an arrow shape
    const double delta = M_PI / 6.0;
    const double angle = atan2(c.y - this->startPoint.y, c.x - this->startPoint.x);

    std::pair<std::vector<Point>, Range> res; // members initialised below
    std::vector<Point>& shape = res.first;

    shape.reserve(doubleEnded ? 9 : 5);

    shape.emplace_back(this->startPoint);

    if (doubleEnded) {
        shape.emplace_back(startPoint.x + arrowDist * cos(angle + delta),
                           startPoint.y + arrowDist * sin(angle + delta));
        shape.emplace_back(startPoint);
        shape.emplace_back(startPoint.x + arrowDist * cos(angle - delta),
                           startPoint.y + arrowDist * sin(angle - delta));
        shape.emplace_back(startPoint);
    }

    shape.emplace_back(c);
    shape.emplace_back(c.x - arrowDist * cos(angle + delta), c.y - arrowDist * sin(angle + delta));
    shape.emplace_back(c);
    shape.emplace_back(c.x - arrowDist * cos(angle - delta), c.y - arrowDist * sin(angle - delta));

    auto [minX, maxX] = std::minmax_element(shape.begin(), shape.end(), [](auto& p, auto& q) { return p.x < q.x; });
    auto [minY, maxY] = std::minmax_element(shape.begin(), shape.end(), [](auto& p, auto& q) { return p.y < q.y; });
    res.second = Range(minX->x, minY->y, maxX->x, maxY->y);

    return res;
}
