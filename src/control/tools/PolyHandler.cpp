#include "PolyHandler.h"

#include <cmath>

#include "control/Control.h"
#include "gui/XournalView.h"
#include "undo/InsertUndoAction.h"


PolyHandler::PolyHandler(XournalView* xournal, XojPageView* redrawable, const PageRef& page, bool flipShift,
                               bool flipControl):
        BaseStrokeHandler(xournal, redrawable, page, flipShift, flipControl) {}

PolyHandler::~PolyHandler() = default;


void PolyHandler::drawShape(Point& c, const PositionInputData& pos) {
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
        double center_x = this->startPoint.x;
        double center_y = this->startPoint.y;

        if (npts < 24) {
            npts = 24;  // min. number of points
        }

        // remove previous points
        stroke->deletePointsFrom(0);
        if (!modShift & !modControl){
            // x²
            for (int j = 0; j <= npts; j++) {
                double x = -2.5 +j*5/npts;
                double y = x*x;
                stroke->addPoint(Point(center_x+x*width/2.5,center_y+y*height/6.25));
            }
        } else if (!modShift & modControl){
            // x³
            for (int j = 0; j <= npts; j++) {
                double x = -2.5 +j*5/npts;
                double y = x*x*x;
                stroke->addPoint(Point(center_x+x*width/2.5,center_y+y*height/15.625));
            }
        } else if (modShift & !modControl){
            // x⁴
            for (int j = 0; j <= npts; j++) {
                double x = -2.5 +j*5/npts;
                double y = x*x*x*x;
                stroke->addPoint(Point(center_x+x*width/2.5,center_y+y*height/39.0625));
            }
        } else {
            // x⁵
            for (int j = 0; j <= npts; j++) {
                double x = -2.5 +j*5/npts;
                double y = x*x*x*x*x;
                stroke->addPoint(Point(center_x+x*width/2.5,center_y+y*height/97.65625));
            }
        }

    }
}
