#include "InputHandler.h"

#include <cmath>    // for hypot
#include <cstddef>  // for size_t

#include <glib.h>  // for g_get_monotonic_time

#include "control/AudioController.h"  // for AudioController
#include "control/Control.h"          // for Control
#include "control/ToolEnums.h"        // for TOOL_ERASER, TOOL_HIGHLIGHTER
#include "control/ToolHandler.h"      // for ToolHandler
#include "gui/XournalView.h"          // for XournalView
#include "model/Point.h"              // for Point, Point::NO_PRESSURE
#include "model/Stroke.h"             // for Stroke, STROKE_TOOL_ERASER, STR...
#include "util/Color.h"               // for Color

#include "filesystem.h"  // for path

InputHandler::InputHandler(XournalView* xournal, XojPageView* redrawable, const PageRef& page):
        xournal(xournal), redrawable(redrawable), page(page), stroke(nullptr) {}

InputHandler::~InputHandler() = default;

/**
 * @return Current editing stroke
 */
auto InputHandler::getStroke() -> Stroke* { return stroke; }

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
            fs::path audioFilename = xournal->getControl()->getAudioController()->getAudioFilename();
            size_t sttime = xournal->getControl()->getAudioController()->getStartTime();
            size_t milliseconds = ((g_get_monotonic_time() / 1000) - sttime);
            stroke->setTimestamp(milliseconds);
            stroke->setAudioFilename(audioFilename);
        }
    } else if (h->getToolType() == TOOL_HIGHLIGHTER) {
        stroke->setToolType(STROKE_TOOL_HIGHLIGHTER);
        p.z = Point::NO_PRESSURE;
    } else if (h->getToolType() == TOOL_ERASER) {
        stroke->setToolType(STROKE_TOOL_ERASER);
        stroke->setColor(Color(0xffffffU));
        p.z = Point::NO_PRESSURE;
    }

    stroke->addPoint(p);
}

auto InputHandler::validMotion(Point p, Point q) -> bool {
    return hypot(p.x - q.x, p.y - q.y) >= PIXEL_MOTION_THRESHOLD;
}
