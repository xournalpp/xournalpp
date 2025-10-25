#include "ExpHandler.h"

#include <cmath>
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

    if (npts < 24) {
        npts = 24;  // min. number of points
    }

    std::pair<std::vector<Point>, Range> res; // members initialised below
    std::vector<Point>& shape = res.first;
    Range& rg = res.second;
    shape.reserve(static_cast<int>(npts));
    if(!modControl){
        //exp(x)
        for (int j = 0; j <= npts; j++) {
            double x = -4.0+j*6/npts;
            double y = exp(x);
            Point p(start_x+x*width/2., start_y+(y-1.)*height/(M_E*M_E-1.));
            rg.addPoint(p.x, p.y);
            shape.emplace_back(p);
        }
    } else {
        //ln(x)
        for (int j = 0; j <= npts; j++) {
            double x = 0.1+j*6.9/npts;
            double y = log2(x);
            Point p(start_x+(x-1)*width/6, start_y+y*height/log2(7.));
            rg.addPoint(p.x, p.y);
            shape.emplace_back(p);
        }
    }
    return res;
}
