#include "RulerInputHandler.h"

#include <cmath>
#include <optional>

#include <gdk/gdkkeysyms.h>

#include "control/Control.h"
#include "control/RulerController.h"
#include "control/ToolHandler.h"
#include "control/actions/ActionDatabase.h"
#include "control/settings/ButtonConfig.h"
#include "control/settings/Settings.h"
#include "control/settings/SettingsEnums.h"
#include "gui/PageView.h"
#include "gui/XournalView.h"
#include "gui/inputdevices/InputEvents.h"
#include "gui/inputdevices/InputUtils.h"
#include "model/Ruler.h"
#include "model/XojPage.h"

namespace {
constexpr double SNAP_EDGE_TOLERANCE_PIXELS = 12.0;

auto resolveButtonForEvent(InputEvent const& event) -> std::optional<Button> {
    if (event.deviceClass == INPUT_DEVICE_MOUSE) {
        switch (event.button) {
            case 2:
                return Button::BUTTON_MOUSE_MIDDLE;
            case 3:
                return Button::BUTTON_MOUSE_RIGHT;
#if defined(_WIN32) || defined(__APPLE__)
            case 4:
                return Button::BUTTON_MOUSE_4;
            case 5:
                return Button::BUTTON_MOUSE_5;
#else
            case 8:
                return Button::BUTTON_MOUSE_4;
            case 9:
                return Button::BUTTON_MOUSE_5;
#endif
            default:
                return Button::BUTTON_MOUSE_LEFT;
        }
    }
    if (event.deviceClass == INPUT_DEVICE_PEN) {
        if (event.button == 2 || (event.state & GDK_BUTTON2_MASK)) {
            return Button::BUTTON_STYLUS_ONE;
        }
        if (event.button == 3 || (event.state & GDK_BUTTON3_MASK)) {
            return Button::BUTTON_STYLUS_TWO;
        }
    }
    return std::nullopt;
}
}  // namespace

RulerInputHandler::RulerInputHandler(XournalView* xournalView, GeometryToolController* controller):
        GeometryToolInputHandler(xournalView, controller) {}
RulerInputHandler::~RulerInputHandler() noexcept = default;

auto RulerInputHandler::keyPressed(KeyEvent const& event) -> bool {
    constexpr GdkModifierType DISALLOWED_TOGGLE_MODIFIERS = static_cast<GdkModifierType>(
            GDK_SHIFT_MASK | GDK_CONTROL_MASK | GDK_MOD1_MASK | GDK_SUPER_MASK | GDK_META_MASK);
    switch (event.keyval) {
        case GDK_KEY_r:
        case GDK_KEY_R:
            if ((event.state & DISALLOWED_TOGGLE_MODIFIERS) == 0) {
                xournal->getControl()->getActionDatabase()->fireActivateAction(Action::TRAD_RULER);
                return true;
            }
            return false;
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

void RulerInputHandler::resetPointerSequence() {
    handScrolling = false;
    activeDeviceId = {};
    activeButton = 0;
    if (buttonToolApplied) {
        buttonToolApplied = false;
        auto* toolHandler = xournal->getControl()->getToolHandler();
        if (toolHandler->pointActiveToolToToolbarTool()) {
            toolHandler->fireToolChanged();
        }
    }
}

auto RulerInputHandler::handlePointer(InputEvent const& event) -> bool {
    const auto coords = getCoords(event);
    auto* ruler = static_cast<RulerController*>(controller);
    const auto p = ruler->posRelToRuler(coords.x, coords.y);

    if (event.type == GRAB_BROKEN_EVENT) {
        if ((ruler->existsStroke() || handScrolling) && event.deviceId == activeDeviceId) {
            if (ruler->existsStroke()) {
                ruler->cancelStroke();
            }
            resetPointerSequence();
            return true;
        }
        return false;
    }

    if (ruler->existsStroke()) {
        if (event.deviceId != activeDeviceId) {
            return false;
        }
        if (event.type == MOTION_EVENT) {
            ruler->updateStroke(p.x);
            return true;
        }
        if (event.type == BUTTON_RELEASE_EVENT && (activeButton == 0 || event.button == activeButton)) {
            ruler->updateStroke(p.x);
            ruler->finalizeStroke();
            resetPointerSequence();
            return true;
        }
        return false;
    }

    if (handScrolling) {
        if (event.deviceId != activeDeviceId) {
            return false;
        }
        if (event.type == MOTION_EVENT) {
            scrollMotion(event);
            return true;
        }
        if (event.type == BUTTON_RELEASE_EVENT && (activeButton == 0 || event.button == activeButton)) {
            resetPointerSequence();
            return true;
        }
        return false;
    }

    if (event.type != BUTTON_PRESS_EVENT) {
        return false;
    }

    auto* control = xournal->getControl();
    auto* settings = control->getSettings();
    auto* toolHandler = control->getToolHandler();
    if (event.deviceClass == INPUT_DEVICE_PEN && event.button != 1 &&
        !settings->getInputSystemTPCButtonEnabled()) {
        return false;
    }

    const auto mappedButton = resolveButtonForEvent(event);
    ToolType toolType = toolHandler->getToolType();
    if (mappedButton) {
        if (auto* cfg = settings->getButtonConfig(*mappedButton); cfg && cfg->getAction() != TOOL_NONE) {
            toolType = cfg->getAction();
        }
    }

    if (toolType != TOOL_PEN && toolType != TOOL_HIGHLIGHTER && toolType != TOOL_HAND) {
        return false;
    }

    if (toolType == TOOL_HAND) {
        if (ruler->isInsideGeometryTool(coords.x, coords.y)) {
            if (mappedButton && InputUtils::applyButton(toolHandler, settings, *mappedButton)) {
                buttonToolApplied = true;
                toolHandler->fireToolChanged();
            }
            sequenceStart(event);
            handScrolling = true;
            activeDeviceId = event.deviceId;
            activeButton = event.button;
            return true;
        }
        return false;
    }

    const auto page = controller->getPage();
    if (!page || coords.x < 0. || coords.y < 0. || coords.x > page->getWidth() || coords.y > page->getHeight()) {
        return false;
    }

    if (ruler->isNearRulerEdge(coords.x, coords.y, SNAP_EDGE_TOLERANCE_PIXELS, xournal->getZoom())) {
        if (mappedButton && InputUtils::applyButton(toolHandler, settings, *mappedButton)) {
            buttonToolApplied = true;
            toolHandler->fireToolChanged();
        }
        activeDeviceId = event.deviceId;
        activeButton = event.button;
        ruler->createStroke(p.x);
        return true;
    }

    return false;
}

auto RulerInputHandler::getMinHeight() const -> double { return 10.; }
auto RulerInputHandler::getMaxHeight() const -> double { return 100.; }
