#include "ArrowHandler.h"

#include <cmath>

#include "control/Control.h"
#include "gui/XournalView.h"
#include "undo/InsertUndoAction.h"

ArrowHandler::ArrowHandler(XournalView* xournal, XojPageView* redrawable, const PageRef& page):
        BaseStrokeHandler(xournal, redrawable, page) {}

ArrowHandler::~ArrowHandler() = default;

void ArrowHandler::drawShape(Point& c, const PositionInputData& pos) {
    this->currPoint = c;  // in case redrawn by keypress event in base class.

    /**
     * Snap first point to grid (if enabled)
     */
    bool altDown = pos.isAltDown();

    Point firstPoint = snappingHandler.snapToGrid(stroke->getPoint(0), altDown);
    stroke->setFirstPoint(firstPoint.x, firstPoint.y);


    int count = stroke->getPointCount();
    if (count < 1) {
        stroke->addPoint(c);
    } else {
        // disable this to see such a cool "serrate brush" effect
        if (count > 4) {
            // remove previous points
            stroke->deletePoint(4);
            stroke->deletePoint(3);
            stroke->deletePoint(2);
            stroke->deletePoint(1);
        }

        c = snappingHandler.snap(c, firstPoint, altDown);

        // We've now computed the line points for the arrow
        // so we just have to build the head

        // set up the size of the arrowhead to be 7x the thickness of the line
        double arrowDist = xournal->getControl()->getToolHandler()->getThickness() * 7.0;

        // an appropriate delta is Pi/3 radians for an arrow shape
        double delta = M_PI / 6.0;
        double angle = atan2(c.y - firstPoint.y, c.x - firstPoint.x);
        stroke->addPoint(c);
        stroke->addPoint(Point(c.x - arrowDist * cos(angle + delta), c.y - arrowDist * sin(angle + delta)));
        stroke->addPoint(c);
        stroke->addPoint(Point(c.x - arrowDist * cos(angle - delta), c.y - arrowDist * sin(angle - delta)));
    }
}
