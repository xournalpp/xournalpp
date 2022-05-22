#include "CoordinateSystemHandler.h"

#include <cmath>

#include "control/Control.h"
#include "gui/XournalView.h"
#include "undo/InsertUndoAction.h"

CoordinateSystemHandler::CoordinateSystemHandler(XournalView* xournal, XojPageView* redrawable, const PageRef& page,
                                                 bool flipShift, bool flipControl):
        BaseStrokeHandler(xournal, redrawable, page, flipShift, flipControl) {}

CoordinateSystemHandler::~CoordinateSystemHandler() = default;

/**
 * Draw a Cartesian coordinate system.
 *
 * @param currentPoint The current point the mouse cursor is pointing to.
 * @param shiftDown Boolean to indicate if "shift" is currently pressed.
 *                  It is currently not used.
 */
auto CoordinateSystemHandler::createShape(const PositionInputData& pos) -> std::vector<Point> {
    /**
     * Snap point to grid (if enabled)
     */
    Point c = snappingHandler.snapToGrid(this->currPoint, pos.isAltDown());

    double width = c.x - this->startPoint.x;
    double height = c.y - this->startPoint.y;

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

        const Point& p1 = this->startPoint;

        if (!this->modControl)  // draw out from starting point
        {
            return {p1, Point(p1.x, p1.y + height), Point(p1.x + width, p1.y + height)};
        } else  // Control is down
        {
            return {Point(p1.x, p1.y + height), p1, Point(p1.x + width, p1.y)};
        }
}
