#include "ExpHandler.h"

#include <math.h>
#include "control/Control.h"
#include "control/settings/Settings.h"             // for Settings
#include "gui/XournalView.h"
#include "undo/InsertUndoAction.h"


ExpHandler::ExpHandler(Control* control, const PageRef& page, bool flipShift,
                                                 bool flipControl):
        BaseShapeHandler(control, page, flipShift, flipControl) {}

ExpHandler::~ExpHandler() = default;


auto ExpHandler::createShape(bool isAltDown, bool isShiftDown, bool isControlDown)
        -> std::pair<std::vector<Point>, Range> {
    Point c = snappingHandler.snapToGrid(this->currPoint, isAltDown);

    double width = c.x - this->startPoint.x;
    double height = c.y - this->startPoint.y;

    this->modControl = isControlDown;

    Settings* settings = control->getSettings();
    if (settings->getDrawDirModsEnabled()) {
        // change modifiers based on draw dir
        this->modifyModifiersByDrawDir(width, height, true);
    }

    double npts = static_cast<int>(std::abs(width * 2.0));
    double start_x = this->startPoint.x;
    double start_y = this->startPoint.y;

    double f = width/30.;

    if (npts < 24) {
        npts = 24;  // min. number of points
    }

    std::pair<std::vector<Point>, Range> res; // members initialised below
    std::vector<Point>& shape = res.first;
    Range& rg = res.second;
    shape.reserve(static_cast<int>(npts));
    if(isShiftDown){
        //1-exp(-x)
        double sign = c.y>start_y?1.:-1;
        double a = height*height/400.;
        double ymax = 1.-exp(-((npts-1)*f/npts)*a);
        for (int j = 0; j <= npts; j++) {
            double x = j*f/npts;
            double y = 1.-exp(-x*a);
            Point p(start_x+x*30., start_y+y*80.*sign/ymax);
            rg.addPoint(p.x, p.y);
            shape.emplace_back(p);
        }
    } else if (isControlDown) {
        //ln(x)
        for (int j = 0; j <= npts; j++) {
            double x = 0.1+j*6.9/npts;
            double y = log2(x);
            Point p(start_x+(x-1)*width/6, start_y+y*height/log2(7.));
            rg.addPoint(p.x, p.y);
            shape.emplace_back(p);
        }
    } else {
        //exp(x)
        double sign = c.y>start_y?1.:-1;
        double a = height*height/400.;
        double ymax = exp((1.-1./npts)*f*a)-1;
        for (int j = 0; j <= npts; j++) {
            double x = j*f/npts;
            double y = exp(x*a);
            Point p(start_x+x*30., start_y+(y-1.)*80.*sign/ymax);
            rg.addPoint(p.x, p.y);
            shape.emplace_back(p);
        }
    }
    return res;
}
