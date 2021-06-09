#include "CoordinateSystemHandler.h"

#include <cmath>

#include "control/Control.h"
#include "gui/XournalView.h"
#include "undo/InsertUndoAction.h"

CoordinateSystemHandler::CoordinateSystemHandler(XournalView* xournal, XojPageView* redrawable, const PageRef& page,
                                                 bool flipShift, bool flipControl):
        PLShapeHandler(xournal, redrawable, page, flipShift, flipControl) {}

CoordinateSystemHandler::~CoordinateSystemHandler() = default;

/**
 * Draw a Cartesian coordinate system.
 *
 * @param currentPoint The current point the mouse cursor is pointing to.
 * @param shiftDown Boolean to indicate if "shift" is currently pressed.
 *                  It is currently not used.
 */
void CoordinateSystemHandler::drawShape(Point& c, const PositionInputData& pos) {
    /**
     * Snap point to grid (if enabled)
     */
    c = snappingHandler.snapToGrid(c, pos.isAltDown());

    double width = c.x - this->startPoint.x;
    double height = c.y - this->startPoint.y;

    this->currPoint = c;

    this->modShift = pos.isShiftDown();
    this->modControl = pos.isControlDown();

    Settings* settings = xournal->getControl()->getSettings();
    if (settings->getDrawDirModsEnabled())  // change modifiers based on draw dir
    {
        this->modifyModifiersByDrawDir(width, height, true);
    }

    if (this->modShift)  // make square
    {
        int signW = width > 0 ? 1 : -1;
        int signH = height > 0 ? 1 : -1;
        width = std::max(width * signW, height * signH) * signW;
        height = (width * signW) * signH;
    }

    Point p1 = this->startPoint;

    path->clear();  // delete previous points

    if (!this->modControl)  // draw out from starting point
    {
        path->setFirstPoint(p1);
        path->addLineSegmentTo(Point(p1.x, p1.y + height));
        path->addLineSegmentTo(Point(p1.x + width, p1.y + height));
    } else  // Control is down
    {
        path->setFirstPoint(Point(p1.x, p1.y + height));
        path->addLineSegmentTo(p1);
        path->addLineSegmentTo(Point(p1.x + width, p1.y));
    }
}
