#include "ArrowHandler.h"

#include <cmath>

#include "control/Control.h"
#include "gui/XournalView.h"
#include "undo/InsertUndoAction.h"

ArrowHandler::ArrowHandler(XournalView* xournal, XojPageView* redrawable, const PageRef& page):
        BaseStrokeHandler(xournal, redrawable, page) {}

ArrowHandler::~ArrowHandler() = default;

auto ArrowHandler::createShape(const PositionInputData& pos) -> std::vector<Point> {
    Point c = snappingHandler.snap(this->currPoint, this->startPoint, pos.isAltDown());

    // We've now computed the line points for the arrow
    // so we just have to build the head

    // set up the size of the arrowhead to be 7x the thickness of the line
    double arrowDist = xournal->getControl()->getToolHandler()->getThickness() * 7.0;

    // an appropriate delta is Pi/3 radians for an arrow shape
    double delta = M_PI / 6.0;
    double angle = atan2(c.y - this->startPoint.y, c.x - this->startPoint.x);

    std::vector<Point> shape;
    shape.reserve(5);

    shape.emplace_back(this->startPoint);

    shape.emplace_back(c);
    shape.emplace_back(Point(c.x - arrowDist * cos(angle + delta), c.y - arrowDist * sin(angle + delta)));
    shape.emplace_back(c);
    shape.emplace_back(Point(c.x - arrowDist * cos(angle - delta), c.y - arrowDist * sin(angle - delta)));
    return shape;
}
