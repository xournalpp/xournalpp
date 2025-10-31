#include "PolyNegHandler.h"

#include <cmath>
#include <math.h>
#include "control/Control.h"
#include "control/settings/Settings.h"             // for Settings
#include "gui/XournalView.h"
#include "undo/InsertUndoAction.h"


PolyNegHandler::PolyNegHandler(Control* control, const PageRef& page, bool flipShift,
                                                 bool flipControl):
        BaseShapeHandler(control, page, flipShift, flipControl) {}

PolyNegHandler::~PolyNegHandler() = default;


auto PolyNegHandler::createShape(bool isAltDown, bool isShiftDown, bool isControlDown)
        -> std::pair<std::vector<Point>, Range> {
    Point c = snappingHandler.snapToGrid(this->currPoint, isAltDown);

    double width = c.x - this->startPoint.x;
    double height = c.y - this->startPoint.y;

    this->modShift = isShiftDown;
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
    if(!modShift & !modControl){
        // 1/x
        for (int j = 0; j <= npts; j++) {
            double x = 0.25 + j*3.75/npts;
            double y = 1/x;
            Point p(start_x+(x-1)*width/3, start_y-(y-1.)*height/0.75);
            rg.addPoint(p.x, p.y);
            shape.emplace_back(p);
        }
    } else if (!modShift & modControl){
        // sqrt(x)
        for (int j = 0; j <= npts; j++) {
            double x = 0 + j*5/npts;
            double y = sqrt(x);
            Point p(start_x+x*width/5, start_y+y*height/sqrt(5));
            rg.addPoint(p.x, p.y);
            shape.emplace_back(p);
        }
    } else if (modShift & !modControl){
        // 1/x²
        for (int j = 0; j <= npts; j++) {
            double x = 0.3 + j*4.7/npts;
            double y = 1/(x*x);
            Point p(start_x+(x-1)*width/4, start_y-(y-1)*height/0.96);
            rg.addPoint(p.x, p.y);
            shape.emplace_back(p);
        }
    } else {
        // 1/x³
        for (int j = 0; j <= npts; j++) {
            double x = 0.4 + j*2.7/npts;
            double y = 1/(x*x*x);
            Point p(start_x+(x-1)*width/2, start_y-(y-1)*height/(26./27.));
            rg.addPoint(p.x, p.y);
            shape.emplace_back(p);
        }
    }
    return res;
}
