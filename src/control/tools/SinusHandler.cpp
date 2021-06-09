#include "SinusHandler.h"

#include <cmath>
#include <math.h>
#include "control/Control.h"
#include "gui/XournalView.h"
#include "undo/InsertUndoAction.h"


SinusHandler::SinusHandler(XournalView* xournal, XojPageView* redrawable, const PageRef& page, bool flipShift,
                               bool flipControl):
        BaseStrokeHandler(xournal, redrawable, page, flipShift, flipControl) {}

SinusHandler::~SinusHandler() = default;


void SinusHandler::drawShape(Point& c, const PositionInputData& pos) {
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
        double start_y = this->startPoint.y;

        if (npts < 24) {
            npts = 24;  // min. number of points
        }

        // remove previous points
        stroke->deletePointsFrom(0);
        double period;
        if(!modShift & !modControl){
            period = 2;
        } else if (!modShift & modControl){
            period = 3;
        } else if (modShift & !modControl){
            period = 4;
        } else {
            period = 5;
        }
        for (int j = 0; j <= npts; j++) {
            double x = j*period*M_PI/npts;
            double y = sin(x);
            stroke->addPoint(Point(start_x+x*width/period/M_PI, start_y-y*height));
        }
    }
}
