#include "GaussHandler.h"

#include <cmath>

#include "control/Control.h"
#include "control/settings/Settings.h"             // for Settings
#include "gui/XournalView.h"
#include "undo/InsertUndoAction.h"


GaussHandler::GaussHandler(Control* control, const PageRef& page, bool flipShift,
                                                 bool flipControl):
        BaseShapeHandler(control, page, flipShift, flipControl) {}

GaussHandler::~GaussHandler() = default;


auto GaussHandler::createShape(bool isAltDown, bool isShiftDown, bool isControlDown)
        -> std::pair<std::vector<Point>, Range> {
    /**
     * Snap point to grid (if enabled - Alt key pressed will toggle)
     */
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
    double start_y = this->startPoint.y + height;

    if (npts < 24) {
        npts = 24;  // max. number of points
    }

    std::pair<std::vector<Point>, Range> res; // members initialised below
    std::vector<Point>& shape = res.first;
    Range& rg = res.second;
    shape.reserve(static_cast<int>(npts));
    for (int j = 0; j <= npts; j++) {
        double x = -2.5 + j*5/npts;
        double y = exp(-x*x);
        Point p(start_x+x*width/2.5, start_y - y*height);
        rg.addPoint(p.x, p.y);
        shape.emplace_back(p);
    }
    return res;
}
