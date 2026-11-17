#include "RulerInputHandler.h"

#include <cmath>

#include <gdk/gdkkeysyms.h>

#include "control/Control.h"
#include "control/RulerController.h"
#include "control/ToolHandler.h"
#include "gui/XournalView.h"
#include "gui/inputdevices/InputEvents.h"
#include "model/Ruler.h"

RulerInputHandler::RulerInputHandler(XournalView* xournalView, GeometryToolController* controller):
        GeometryToolInputHandler(xournalView, controller) {}
RulerInputHandler::~RulerInputHandler() noexcept = default;

auto RulerInputHandler::keyPressed(KeyEvent const& event) -> bool {
    switch (event.keyval) {
        case GDK_KEY_bracketleft:
            controller->rotate(-M_PI / 180.);
            return true;
        case GDK_KEY_bracketright:
            controller->rotate(M_PI / 180.);
            return true;
        default:
            return GeometryToolInputHandler::keyPressed(event);
    }
}

auto RulerInputHandler::handlePointer(InputEvent const& event) -> bool {
    const auto coords = getCoords(event);
    auto* ruler = static_cast<RulerController*>(controller);
    const auto toolType = xournal->getControl()->getToolHandler()->getToolType();
    if (toolType != TOOL_PEN && toolType != TOOL_HIGHLIGHTER && toolType != TOOL_HAND) {
        return false;
    }

    if (toolType == TOOL_HAND) {
        if (event.type == BUTTON_PRESS_EVENT && ruler->isInsideGeometryTool(coords.x, coords.y)) {
            sequenceStart(event);
            handScrolling = true;
            return true;
        }
        if (event.type == MOTION_EVENT && handScrolling) {
            scrollMotion(event);
            return true;
        }
        if (event.type == BUTTON_RELEASE_EVENT && handScrolling) {
            handScrolling = false;
            return true;
        }
        return false;
    }

    const auto p = ruler->posRelToRuler(coords.x, coords.y);
    switch (event.type) {
        case BUTTON_PRESS_EVENT:
            if (ruler->isInsideGeometryTool(coords.x, coords.y) && p.y >= -0.5 && p.y <= 0.5) {
                ruler->createStroke(p.x);
                return true;
            }
            break;
        case MOTION_EVENT:
            if (ruler->existsStroke()) {
                ruler->updateStroke(p.x);
                return true;
            }
            break;
        case BUTTON_RELEASE_EVENT:
            if (ruler->existsStroke()) {
                ruler->finalizeStroke();
                return true;
            }
            break;
        default:
            break;
    }
    return false;
}

auto RulerInputHandler::getMinHeight() const -> double { return 10.; }
auto RulerInputHandler::getMaxHeight() const -> double { return 100.; }
