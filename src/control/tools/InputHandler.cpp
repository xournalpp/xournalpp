#include "InputHandler.h"

#include <cmath>

#include "control/Control.h"
#include "control/shaperecognizer/ShapeRecognizerResult.h"
#include "gui/MainWindow.h"
#include "gui/PageView.h"
#include "gui/XournalView.h"
#include "gui/widgets/XournalWidget.h"
#include "model/Layer.h"
#include "undo/InsertUndoAction.h"
#include "undo/RecognizerUndoAction.h"
#include "util/Rectangle.h"
#include "view/DocumentView.h"

InputHandler::InputHandler(XournalView* xournal, XojPageView* redrawable, const PageRef& page):
        xournal(xournal), redrawable(redrawable), page(page), stroke(nullptr) {}

InputHandler::~InputHandler() = default;

auto InputHandler::getStroke() -> Stroke* { return stroke.get(); }

/**
 * Reset the shape recognizer, only implemented by drawing instances,
 * but needs to be in the base interface.
 */
void InputHandler::resetShapeRecognizer() {
    // Does nothing here. Implemented in the extending classes
}

auto InputHandler::createStroke(Point p, Control* control) -> std::unique_ptr<Stroke> {
    ToolHandler* h = control->getToolHandler();

    auto s = std::make_unique<Stroke>();
    s->setWidth(h->getThickness());
    s->setColor(h->getColor());
    s->setFill(h->getFill());
    s->setLineStyle(h->getLineStyle());

    if (h->getToolType() == TOOL_PEN) {
        s->setToolType(STROKE_TOOL_PEN);

        if (control->getAudioController()->isRecording()) {
            std::string audioFilename = control->getAudioController()->getAudioFilename();
            size_t sttime = control->getAudioController()->getStartTime();
            size_t milliseconds = ((g_get_monotonic_time() / 1000) - sttime);
            s->setTimestamp(milliseconds);
            s->setAudioFilename(audioFilename);
        }
    } else if (h->getToolType() == TOOL_HIGHLIGHTER) {
        s->setToolType(STROKE_TOOL_HIGHLIGHTER);
        p.z = Point::NO_PRESSURE;
    } else if (h->getToolType() == TOOL_ERASER) {
        s->setToolType(STROKE_TOOL_ERASER);
        s->setColor(0xffffffU);
        p.z = Point::NO_PRESSURE;
    }

    s->addPoint(p);
    return s;
}

auto InputHandler::validMotion(Point p, Point q) -> bool {
    return hypot(p.x - q.x, p.y - q.y) >= PIXEL_MOTION_THRESHOLD;
}
