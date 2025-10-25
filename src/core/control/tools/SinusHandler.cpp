#include "SinusHandler.h"

#include <cmath>
#include <math.h>
#include "control/Control.h"
#include "control/settings/Settings.h"             // for Settings
#include "gui/XournalView.h"
#include "undo/InsertUndoAction.h"


SinusHandler::SinusHandler(Control* control, const PageRef& page, bool flipShift,
                                                 bool flipControl):
        BaseShapeHandler(control, page, flipShift, flipControl) {}

SinusHandler::~SinusHandler() = default;


auto SinusHandler::createShape(bool isAltDown, bool isShiftDown, bool isControlDown)
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
    double period;
    if (isShiftDown && isControlDown) {
        period = width/12.;
    } else if (isShiftDown) {
        period = width/24.;
    } else if (isControlDown) {
        period = width/36.;
    } else {
        period = width/48;
    }

    for (int j = 0; j <= npts; j++) {
        double x = j*period*M_PI/npts;
        double y = sin(x);
        Point p(start_x+x*width/period/M_PI, start_y-y*height);
        rg.addPoint(p.x, p.y);
        shape.emplace_back(p);
    }
    return res;
}
