#include "PolyNegHandler.h"

#include <cmath>
#include <math.h>
#include "control/Control.h"
#include "gui/XournalView.h"
#include "undo/InsertUndoAction.h"


PolyNegHandler::PolyNegHandler(XournalView* xournal, XojPageView* redrawable, const PageRef& page, bool flipShift,
                               bool flipControl):
        BaseStrokeHandler(xournal, redrawable, page, flipShift, flipControl) {}

PolyNegHandler::~PolyNegHandler() = default;


void PolyNegHandler::drawShape(Point& c, const PositionInputData& pos) {
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
        if(!modShift & !modControl){
            // 1/x
            for (int j = 0; j <= npts; j++) {
                double x = 0.25 + j*3.75/npts;
                double y = 1/x;
                stroke->addPoint(Point(start_x+(x-1)*width/3, start_y-(y-1.)*height/0.75));
            }
        } else if (!modShift & modControl){
            // sqrt(x)
            for (int j = 0; j <= npts; j++) {
                double x = 0 + j*5/npts;
                double y = sqrt(x);
                stroke->addPoint(Point(start_x+x*width/5, start_y+y*height/sqrt(5)));
            }
        } else if (modShift & !modControl){
            // 1/x²
            for (int j = 0; j <= npts; j++) {
                double x = 0.3 + j*4.7/npts;
                double y = 1/(x*x);
                stroke->addPoint(Point(start_x+(x-1)*width/4, start_y-(y-1)*height/0.96));
            }
        } else {
            // 1/x³
            for (int j = 0; j <= npts; j++) {
                double x = 0.4 + j*2.7/npts;
                double y = 1/(x*x*x);
                stroke->addPoint(Point(start_x+(x-1)*width/2, start_y-(y-1)*height/(26./27.)));
            }
        }

    }
}
