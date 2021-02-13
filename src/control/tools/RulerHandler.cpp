#include "RulerHandler.h"

#include <cmath>

#include "control/Control.h"
#include "gui/XournalView.h"
#include "undo/InsertUndoAction.h"

RulerHandler::RulerHandler(XournalView* xournal, XojPageView* redrawable, const PageRef& page):
        PLShapeHandler(xournal, redrawable, page) {}

RulerHandler::~RulerHandler() = default;

void RulerHandler::drawShape(Point& currentPoint, const PositionInputData& pos) {
    this->currPoint = currentPoint;  // in case redrawn by keypress event in base class.

    /**
     * Snap first point to grid (if enabled)
     */
    bool altDown = pos.isAltDown();

    Point firstPoint = snappingHandler.snapToGrid(path->getFirstKnot(), altDown);
    path->setFirstPoint(firstPoint);

    path->resize(0);  // Remove every segment. Keep the starting point

    Point secondPoint = snappingHandler.snap(currentPoint, firstPoint, altDown);
    path->addLineSegmentTo(secondPoint);
}
