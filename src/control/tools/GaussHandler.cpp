#include "GaussHandler.h"

#include <cmath>

#include "control/Control.h"
#include "gui/XournalView.h"
#include "undo/InsertUndoAction.h"


GaussHandler::GaussHandler(XournalView* xournal, XojPageView* redrawable, const PageRef& page, bool flipShift,
                               bool flipControl):
        BaseStrokeHandler(xournal, redrawable, page, flipShift, flipControl) {}

GaussHandler::~GaussHandler() = default;


void GaussHandler::drawShape(Point& c, const PositionInputData& pos) {
    this->currPoint = c;

    /**
     * Snap point to grid (if enabled - Alt key pressed will toggle)
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



        double npts = static_cast<int>(std::abs(width * 2.0));
        double start_x = this->startPoint.x;
        double start_y = this->startPoint.y + height;

        if (npts < 24) {
            npts = 24;  // min. number of points
        }

        // remove previous points
        stroke->deletePointsFrom(0);
        for (int j = 0; j <= npts; j++) {
            double x = -2.5 + j*5/npts;
            double y = exp(-x*x);
            stroke->addPoint(Point(start_x+x*width/2.5, start_y - y*height));
        }
    }
}
