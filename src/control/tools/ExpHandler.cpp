#include "ExpHandler.h"

#include <cmath>
#include <math.h>
#include "control/Control.h"
#include "gui/XournalView.h"
#include "undo/InsertUndoAction.h"


ExpHandler::ExpHandler(XournalView* xournal, XojPageView* redrawable, const PageRef& page, bool flipShift,
                               bool flipControl):
        BaseStrokeHandler(xournal, redrawable, page, flipShift, flipControl) {}

ExpHandler::~ExpHandler() = default;


void ExpHandler::drawShape(Point& c, const PositionInputData& pos) {
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
        if(!modControl){
            //exp(x)
            for (int j = 0; j <= npts; j++) {
                double x = -4.0+j*6/npts;
                double y = exp(x);
                stroke->addPoint(Point(start_x+x*width/2., start_y+(y-1.)*height/(M_E*M_E-1.)));
            }
        } else {
            //ln(x)
            for (int j = 0; j <= npts; j++) {
                double x = 0.1+j*6.9/npts;
                double y = log2(x);
                stroke->addPoint(Point(start_x+(x-1)*width/6, start_y+y*height/log2(7.)));
            }
        }
    }
}
