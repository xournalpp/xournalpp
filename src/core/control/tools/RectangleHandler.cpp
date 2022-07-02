#include "RectangleHandler.h"

#include <algorithm>  // for max

#include "control/Control.h"                       // for Control
#include "control/settings/Settings.h"             // for Settings
#include "control/tools/BaseShapeHandler.h"        // for BaseShapeHandler
#include "control/tools/SnapToGridInputHandler.h"  // for SnapToGridInputHan...
#include "gui/XournalView.h"                       // for XournalView
#include "gui/inputdevices/PositionInputData.h"    // for PositionInputData
#include "model/Stroke.h"                          // for Stroke

class XojPageView;

RectangleHandler::RectangleHandler(XournalView* xournal, XojPageView* redrawable, const PageRef& page, bool flipShift,
                                   bool flipControl):
        BaseShapeHandler(xournal, redrawable, page, flipShift, flipControl) {}

RectangleHandler::~RectangleHandler() = default;

void RectangleHandler::drawShape(Point& c, const PositionInputData& pos) {
    this->currPoint = c;

    /**
     * Snap point to grid (if enabled)
     */
    c = snappingHandler.snapToGrid(c, pos.isAltDown());

    if (!this->started)  // initialize first point
    {
        this->startPoint = c;
        this->started = true;
        stroke->addPoint(c);  // avoid complaints about <2 points.
    } else {
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

        stroke->deletePointsFrom(0);  // delete previous points

        stroke->addPoint(p1);
        stroke->addPoint(Point(p1.x, p2.y));
        stroke->addPoint(p2);
        stroke->addPoint(Point(p2.x, p1.y));
        stroke->addPoint(p1);
    }
}
