#include "ArrowHandler.h"

#include <cmath>  // for cos, sin, atan2, M_PI

#include "control/Control.h"                       // for Control
#include "control/ToolHandler.h"                   // for ToolHandler
#include "control/tools/BaseShapeHandler.h"        // for BaseShapeHandler
#include "control/tools/SnapToGridInputHandler.h"  // for SnapToGridInputHan...
#include "gui/XournalView.h"                       // for XournalView
#include "gui/inputdevices/PositionInputData.h"    // for PositionInputData
#include "model/Point.h"                           // for Point
#include "model/Stroke.h"                          // for Stroke

class XojPageView;

ArrowHandler::ArrowHandler(XournalView* xournal, XojPageView* redrawable, const PageRef& page, bool doubleEnded):
        BaseShapeHandler(xournal, redrawable, page), doubleEnded(doubleEnded) {}

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
            stroke->deletePointsFrom(1);
        }

        c = snappingHandler.snap(c, firstPoint, altDown);

        // We've now computed the line points for the arrow
        // so we just have to build the head

        // set up the size of the arrowhead to be 7x the thickness of the line
        double arrowDist = xournal->getControl()->getToolHandler()->getThickness() * 7.0;

        // an appropriate delta is Pi/3 radians for an arrow shape
        double delta = M_PI / 6.0;
        double angle = atan2(c.y - firstPoint.y, c.x - firstPoint.x);

        if (doubleEnded) {
            stroke->addPoint(Point(firstPoint.x + arrowDist * cos(angle + delta),
                                   firstPoint.y + arrowDist * sin(angle + delta)));
            stroke->addPoint(firstPoint);
            stroke->addPoint(Point(firstPoint.x + arrowDist * cos(angle - delta),
                                   firstPoint.y + arrowDist * sin(angle - delta)));
            stroke->addPoint(firstPoint);
        }

        stroke->addPoint(c);
        stroke->addPoint(Point(c.x - arrowDist * cos(angle + delta), c.y - arrowDist * sin(angle + delta)));
        stroke->addPoint(c);
        stroke->addPoint(Point(c.x - arrowDist * cos(angle - delta), c.y - arrowDist * sin(angle - delta)));
    }
}
