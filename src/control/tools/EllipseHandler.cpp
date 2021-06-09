#include "EllipseHandler.h"

#include "control/Control.h"
#include "gui/XournalView.h"
#include "undo/InsertUndoAction.h"


EllipseHandler::EllipseHandler(XournalView* xournal, XojPageView* redrawable, const PageRef& page, bool flipShift,
                               bool flipControl):
        BaseStrokeHandler(xournal, redrawable, page, flipShift, flipControl) {}

EllipseHandler::~EllipseHandler() = default;

void EllipseHandler::createPath(const Point& p, bool snapToGrid) {
    Point snapped = snappingHandler.snapToGrid(p, snapToGrid);
    this->startPoint = snapped;
    this->path = std::make_shared<Spline>();
    this->stroke->setPath(this->path);
}

const Path& EllipseHandler::getPath() const { return *path; }

void EllipseHandler::drawShape(Point& c, const PositionInputData& pos) {
    this->currPoint = c;

    /**
     * Snap point to grid (if enabled - Alt key pressed will toggle)
     */
    c = snappingHandler.snapToGrid(c, pos.isAltDown());

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
        width = (this->modControl) ? std::hypot(width, height) : std::max(width * signW, height * signH) * signW;
        height = (width * signW) * signH;
    }

    double radiusX = 0;
    double radiusY = 0;
    double centerX = 0;
    double centerY = 0;

    // set resolution proportional to radius
    if (!this->modControl) {
        radiusX = 0.5 * width;
        radiusY = 0.5 * height;
        centerX = this->startPoint.x + radiusX;
        centerY = this->startPoint.y + radiusY;
    } else {  // control key down, draw centered at cursor
        radiusX = width;
        radiusY = height;
        centerX = this->startPoint.x;
        centerY = this->startPoint.y;
    }
    radiusX = std::abs(radiusX);  //  For bounding box computation
    radiusY = std::abs(radiusY);  //

    path->makeEllipse(Point(centerX, centerY), radiusX, radiusY);

    double strokeWidth = this->stroke->getWidth();
    double halfStrokeWidth = 0.5 * strokeWidth;
    Rectangle<double> thinBB{centerX - radiusX, centerY - radiusY, 2 * radiusX, 2 * radiusY};
    Rectangle<double> thickBB{thinBB.x - halfStrokeWidth, thinBB.y - halfStrokeWidth, thinBB.width + strokeWidth,
                              thinBB.height + strokeWidth};

    Stroke::Attorney::setBoundingBoxes(*(this->stroke), thinBB, thickBB);
}
