#include "RectangleHandler.h"

#include <cmath>

#include "control/Control.h"
#include "gui/XournalView.h"
#include "undo/InsertUndoAction.h"

RectangleHandler::RectangleHandler(XournalView* xournal, XojPageView* redrawable, const PageRef& page, bool flipShift,
                                   bool flipControl):
        PLShapeHandler(xournal, redrawable, page, flipShift, flipControl) {}

RectangleHandler::~RectangleHandler() = default;

void RectangleHandler::drawShape(Point& c, const PositionInputData& pos) {
    this->currPoint = c;

    /**
     * Snap point to grid (if enabled)
     */
    c = snappingHandler.snapToGrid(c, pos.isAltDown());

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

    Point p1;
    if (!this->modControl) {
        p1 = this->startPoint;

    } else  // Control is down - drawing from center
    {
        p1 = Point(this->startPoint.x - width, this->startPoint.y - height);
    }

    Point p2 = Point(this->startPoint.x + width, this->startPoint.y + height);

    path->clear();  // delete previous points

    path->setFirstPoint(p1);
    path->addLineSegmentTo(Point(p1.x, p2.y));
    path->addLineSegmentTo(p2);
    path->addLineSegmentTo(Point(p2.x, p1.y));
    path->addLineSegmentTo(p1);
}
