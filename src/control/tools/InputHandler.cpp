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

#define PIXEL_MOTION_THRESHOLD 0.3

InputHandler::InputHandler(XournalView* xournal, XojPageView* redrawable, const PageRef& page):
        xournal(xournal), redrawable(redrawable), page(page), stroke(nullptr) {}

InputHandler::~InputHandler() = default;

/**
 * @return Current editing stroke
 */
auto InputHandler::getStroke() -> Stroke* { return stroke; }

/**
 * Reset the shape recognizer, only implemented by drawing instances,
 * but needs to be in the base interface.
 */
void InputHandler::resetShapeRecognizer() {
    // Does nothing here. Implemented in the extending classes
}

void InputHandler::createStroke(Point p) {
    ToolHandler* h = xournal->getControl()->getToolHandler();

    stroke = new Stroke();
    stroke->setWidth(h->getThickness());
    stroke->setColor(h->getColor());
    stroke->setFill(h->getFill());
    stroke->setLineStyle(h->getLineStyle());

    if (h->getToolType() == TOOL_PEN) {
        stroke->setToolType(STROKE_TOOL_PEN);

        if (xournal->getControl()->getAudioController()->isRecording()) {
            string audioFilename = xournal->getControl()->getAudioController()->getAudioFilename();
            size_t sttime = xournal->getControl()->getAudioController()->getStartTime();
            size_t milliseconds = ((g_get_monotonic_time() / 1000) - sttime);
            stroke->setTimestamp(milliseconds);
            stroke->setAudioFilename(audioFilename);
        }
    } else if (h->getToolType() == TOOL_HILIGHTER) {
        stroke->setToolType(STROKE_TOOL_HIGHLIGHTER);
    } else if (h->getToolType() == TOOL_ERASER) {
        stroke->setToolType(STROKE_TOOL_ERASER);
        stroke->setColor(0xffffffU);
    }

    stroke->addPoint(p);
}

auto InputHandler::validMotion(Point p, Point q) -> bool {
    return hypot(p.x - q.x, p.y - q.y) >= PIXEL_MOTION_THRESHOLD;
}
