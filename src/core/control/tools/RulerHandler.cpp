#include "RulerHandler.h"

#include "control/tools/BaseShapeHandler.h"        // for BaseShapeHandler
#include "control/tools/SnapToGridInputHandler.h"  // for SnapToGridInputHan...
#include "model/Point.h"                           // for Point

class XournalView;

RulerHandler::RulerHandler(Control* control, const PageRef& page): BaseShapeHandler(control, page) {}

RulerHandler::~RulerHandler() = default;

auto RulerHandler::createShape(bool isAltDown, bool isShiftDown, bool isControlDown)
        -> std::pair<std::vector<Point>, Range> {
    Point secondPoint = snappingHandler.snap(this->currPoint, this->startPoint, isAltDown);
    Range rg(this->startPoint.x, this->startPoint.y);
    rg.addPoint(secondPoint.x, secondPoint.y);
    return {{this->startPoint, secondPoint}, rg};
}
