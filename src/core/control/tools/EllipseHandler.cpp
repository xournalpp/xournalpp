#include "EllipseHandler.h"

#include <algorithm>  // for max
#include <cmath>      // for abs, pow, M_PI, cos

#include "control/Control.h"                       // for Control
#include "control/settings/Settings.h"             // for Settings
#include "control/tools/BaseShapeHandler.h"        // for BaseShapeHandler
#include "control/tools/SnapToGridInputHandler.h"  // for SnapToGridInputHan...
#include "gui/XournalView.h"                       // for XournalView
#include "gui/inputdevices/PositionInputData.h"    // for PositionInputData
#include "model/Stroke.h"                          // for Stroke

class XojPageView;


EllipseHandler::EllipseHandler(XournalView* xournal, XojPageView* redrawable, const PageRef& page, bool flipShift,
                               bool flipControl):
        BaseShapeHandler(xournal, redrawable, page, flipShift, flipControl) {}

EllipseHandler::~EllipseHandler() = default;


void EllipseHandler::drawShape(Point& c, const PositionInputData& pos) {
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

        if (this->modShift)  // make circle
        {
            int signW = width > 0 ? 1 : -1;
            int signH = height > 0 ? 1 : -1;
            width = (this->modControl) ? sqrt(pow(width, 2) + pow(height, 2)) :
                                         std::max(width * signW, height * signH) * signW;
            height = (width * signW) * signH;
        }

        double diameterX = 0;
        double diameterY = 0;
        int npts = 0;
        double center_x = 0;
        double center_y = 0;
        double angle = 0;

        // set resolution proportional to radius
        if (!this->modControl) {
            diameterX = width;
            diameterY = height;
            npts = static_cast<int>(std::abs(diameterX * 2.0));
            center_x = this->startPoint.x + width / 2.0;
            center_y = this->startPoint.y + height / 2.0;
            angle = 0;
        } else {  // control key down, draw centered at cursor
            diameterX = width * 2.0;
            diameterY = height * 2.0;
            npts = static_cast<int>(std::abs(diameterX) + std::abs(diameterY));
            center_x = this->startPoint.x;
            center_y = this->startPoint.y;
            angle = 0;
        }
        if (npts < 24) {
            npts = 24;  // min. number of points
        }

        // remove previous points
        stroke->deletePointsFrom(0);
        for (int j = 0; j <= npts; j++) {
            int i = j % npts;  // so that we end exactly where we started.
            double xp = center_x + diameterX / 2.0 * cos((2 * M_PI * i) / npts + angle + M_PI);
            double yp = center_y + diameterY / 2.0 * sin((2 * M_PI * i) / npts + angle + M_PI);
            stroke->addPoint(Point(xp, yp));
        }
    }
}
