#include "PolyHandler.h"

#include <cmath>

#include "control/Control.h"
#include "control/settings/Settings.h"             // for Settings
#include "gui/XournalView.h"
#include "undo/InsertUndoAction.h"


PolyHandler::PolyHandler(Control* control, const PageRef& page, bool flipShift,
                                                 bool flipControl):
        BaseShapeHandler(control, page, flipShift, flipControl) {}
PolyHandler::~PolyHandler() = default;


auto PolyHandler::createShape(bool isAltDown, bool isShiftDown, bool isControlDown)
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
    double center_x = this->startPoint.x;
    double center_y = this->startPoint.y;

    if (npts < 24) {
        npts = 24;  // min. number of points
    }

    std::pair<std::vector<Point>, Range> res; // members initialised below
    std::vector<Point>& shape = res.first;
    Range& rg = res.second;
    shape.reserve(static_cast<int>(npts));
    if (!modShift & !modControl){
        // x²
        for (int j = 0; j <= npts; j++) {
            double x = -2.5 +j*5/npts;
            double y = x*x;
            stroke->addPoint(Point());
            Point p(center_x+x*width/2.5,center_y+y*height/6.25);
            rg.addPoint(p.x, p.y);
            shape.emplace_back(p);
        }
    } else if (!modShift & modControl){
        // x³
        for (int j = 0; j <= npts; j++) {
            double x = -2.5 +j*5/npts;
            double y = x*x*x;
            Point p(center_x+x*width/2.5,center_y+y*height/15.625);
            rg.addPoint(p.x, p.y);
            shape.emplace_back(p);
        }
    } else if (modShift & !modControl){
        // x⁴
        for (int j = 0; j <= npts; j++) {
            double x = -2.5 +j*5/npts;
            double y = x*x*x*x;
            Point p(center_x+x*width/2.5,center_y+y*height/39.0625);
            rg.addPoint(p.x, p.y);
            shape.emplace_back(p);
        }
    } else {
        // x⁵
        for (int j = 0; j <= npts; j++) {
            double x = -2.5 +j*5/npts;
            double y = x*x*x*x*x;
            Point p(center_x+x*width/2.5,center_y+y*height/97.65625);
            rg.addPoint(p.x, p.y);
            shape.emplace_back(p);
        }
    }
    return res;
}
