#include "InputHandler.h"

#include <cmath>    // for hypot
#include <cstddef>  // for size_t

#include <glib.h>  // for g_get_monotonic_time

#include "control/AudioController.h"  // for AudioController
#include "control/Control.h"          // for Control
#include "control/ToolEnums.h"        // for TOOL_ERASER, TOOL_HIGHLIGHTER
#include "control/ToolHandler.h"      // for ToolHandler
#include "model/Point.h"              // for Point, Point::NO_PRESSURE
#include "model/Stroke.h"             // for Stroke, StrokeTool::ERASER, STR...
#include "util/Color.h"               // for Color

#include "filesystem.h"  // for path

InputHandler::InputHandler(Control* control, const PageRef& page): control(control), page(page) {}

InputHandler::~InputHandler() = default;

auto InputHandler::getStroke() const -> Stroke* { return stroke.get(); }

auto InputHandler::createStroke(Control* control) -> std::unique_ptr<Stroke> {
    ToolHandler* h = control->getToolHandler();

    auto s = std::make_unique<Stroke>();
    s->setWidth(h->getThickness());
    s->setColor(h->getColor());
    s->setFill(h->getFill());
    s->setLineStyle(h->getLineStyle());

    if (h->getToolType() == TOOL_PEN) {
        s->setToolType(StrokeTool::PEN);

        if (control->getAudioController()->isRecording()) {
            fs::path audioFilename = control->getAudioController()->getAudioFilename();
            size_t sttime = control->getAudioController()->getStartTime();
            size_t milliseconds = ((g_get_monotonic_time() / 1000) - sttime);
            s->setTimestamp(milliseconds);
            s->setAudioFilename(audioFilename);
        }
    } else if (h->getToolType() == TOOL_HIGHLIGHTER) {
        s->setToolType(StrokeTool::HIGHLIGHTER);
    } else if (h->getToolType() == TOOL_ERASER) {
        s->setToolType(StrokeTool::ERASER);
        s->setColor(Colors::white);
    }

    return s;
}

auto InputHandler::validMotion(Point p, Point q) -> bool {
    return std::hypot(p.x - q.x, p.y - q.y) >= PIXEL_MOTION_THRESHOLD;
}
