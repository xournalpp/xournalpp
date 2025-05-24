#include "RulerHandler.h"

#include "control/tools/BaseShapeHandler.h"        // for BaseShapeHandler
#include "control/tools/SnapToGridInputHandler.h"  // for SnapToGridInputHan...
#include "model/Point.h"                           // for Point
#include "model/path/PiecewiseLinearPath.h"

class XournalView;

RulerHandler::RulerHandler(Control* control, const PageRef& page): BaseShapeHandler(control, page) {}

RulerHandler::~RulerHandler() = default;

auto RulerHandler::createShape(bool isAltDown, bool isShiftDown, bool isControlDown)
        -> std::pair<std::shared_ptr<Path>, Range> {
    Point secondPoint = snappingHandler.snap(this->currPoint, this->startPoint, isAltDown);
    Range rg(this->startPoint.x, this->startPoint.y);
    rg.addPoint(secondPoint.x, secondPoint.y);
    return {std::make_shared<PiecewiseLinearPath>(this->startPoint, secondPoint), rg};
}
