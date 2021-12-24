#include "RulerHandler.h"

#include <cmath>

#include "control/Control.h"
#include "gui/XournalView.h"
#include "undo/InsertUndoAction.h"

RulerHandler::RulerHandler(XournalView* xournal, XojPageView* redrawable, const PageRef& page):
        BaseStrokeHandler(xournal, redrawable, page) {}

RulerHandler::~RulerHandler() = default;

void RulerHandler::drawShape(Point& currentPoint, const PositionInputData& pos) {
    this->currPoint = currentPoint;  // in case redrawn by keypress event in base class.

    /**
     * Snap first point to grid (if enabled)
     */
    bool altDown = pos.isAltDown();

    Point firstPoint = snappingHandler.snapToGrid(stroke->getPoint(0), altDown);
    stroke->setFirstPoint(firstPoint.x, firstPoint.y);

    if (stroke->getPointCount() < 2) {
        stroke->addPoint(currentPoint);
    } else {
        Point secondPoint = snappingHandler.snap(currentPoint, firstPoint, altDown);
        stroke->setLastPoint(secondPoint.x, secondPoint.y);
    }
}
