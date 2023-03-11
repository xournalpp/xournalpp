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

    // an appropriate opening angle 2*delta is Pi/3 radians for an arrow shape
    double delta = M_PI / 6.0;
    // We use different slimness regimes for proper sizing:
    const double THICK1 = 7, THICK3 = 1.6;
    const double LENGTH2 = 0.4, LENGTH4 = (doubleEnded ? 0.5 : 0.8);
    // set up the size of the arrow head to be THICK1 x the thickness of the line
    double arrowDist = thickness * THICK1;
    // but not too large compared to the line length
    if (slimness >= THICK1 / LENGTH2) {
        // arrow head is not too long compared to the line length (regime 1)
    } else if (slimness >= THICK3 / LENGTH2) {
        // arrow head is not too short compared to the thickness (regime 2)
        arrowDist = lineLength * LENGTH2;
    } else if (slimness >= THICK3 / LENGTH4) {
        // arrow head is not too thick compared to the line length (regime 3)
        arrowDist = thickness * THICK3;
        // help visibility by widening the angle
        delta = (1 + (slimness - THICK3 / LENGTH2) / (THICK3 / LENGTH4 - THICK3 / LENGTH2)) *  M_PI / 6.0;
        // which allows to shorten the tips and keep the horizonzal distance
        arrowDist *= sin(M_PI / 6.0) / sin(delta);
    } else {
        // shrinking down gracefully (regime 4)
        arrowDist = lineLength * LENGTH4;
        delta = M_PI / 3.0;
        arrowDist *= sin(M_PI / 6.0) / sin(M_PI / 3.0);
    }

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
