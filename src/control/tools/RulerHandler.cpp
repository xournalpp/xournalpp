#include "RulerHandler.h"

#include <cmath>

#include "control/Control.h"
#include "gui/XournalView.h"
#include "undo/InsertUndoAction.h"

RulerHandler::RulerHandler(XournalView* xournal, XojPageView* redrawable, const PageRef& page):
        BaseStrokeHandler(xournal, redrawable, page) {}

RulerHandler::~RulerHandler() = default;

auto RulerHandler::createShape(const PositionInputData& pos) -> std::vector<Point> {
    Point secondPoint = snappingHandler.snap(this->currPoint, this->startPoint, pos.isAltDown());
    return {this->startPoint, secondPoint};
}
