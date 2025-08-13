#include "GeometryToolInputHandler.h"

#include <algorithm>  // for max
#include <cmath>      // for atan2, abs, cos, remainder
#include <memory>     // for __shared_ptr_access, shar...

#include <gdk/gdkkeysyms.h>  // for GDK_KEY_Down, GDK_KEY_Left
#include <glib.h>            // for g_warning

#include "control/Control.h"                 // for Control
#include "control/GeometryToolController.h"  // for GeometryToolController
#include "control/KeyBindingsGroup.h"        // for KeyBinding, KeyBindingsGroup
#include "control/ToolEnums.h"               // for TOOL_HAND, TOOL_HIGHLIGHTER
#include "control/ToolHandler.h"             // for ToolHandler
#include "control/settings/Settings.h"       // for Settings
#include "gui/XournalView.h"                 // for XournalView
#include "gui/inputdevices/InputContext.h"   // for InputContext::DeviceType
#include "gui/inputdevices/InputEvents.h"    // for InputEvent, BUTTON_PRESS_...
#include "model/Document.h"                  // for Document
#include "model/Element.h"                   // for Element, ELEMENT_STROKE
#include "model/GeometryTool.h"              // for HALF_CM
#include "model/Layer.h"                     // for Layer
#include "model/Snapping.h"                  // for distanceLine
#include "model/Stroke.h"                    // for Stroke
#include "model/XojPage.h"                   // for XojPage

constexpr double MOVE_AMOUNT = HALF_CM / 2.0;
constexpr double MOVE_AMOUNT_SMALL = HALF_CM / 20.0;
constexpr int ROTATE_AMOUNT = 50;                                   ///< in 0.1 degrees
constexpr int ROTATE_AMOUNT_SMALL = 2;                              ///< in 0.1 degrees
constexpr int SCALE_AMOUNT = 110;                                   ///< in %
constexpr int SCALE_AMOUNT_SMALL = 101;                             ///< in %
constexpr double SNAPPING_DISTANCE_TOLERANCE = 5.0;                 // pt
constexpr double SNAPPING_ROTATION_TOLERANCE = 3.0 * M_PI / 180.0;  // rad
constexpr double ZOOM_DISTANCE_MIN = 0.01;

GeometryToolInputHandler::GeometryToolInputHandler(XournalView* xournal, GeometryToolController* controller):
        xournal(xournal), controller(controller) {}

GeometryToolInputHandler::~GeometryToolInputHandler() = default;

auto GeometryToolInputHandler::handle(InputEvent const& event) -> bool {
    if (xournal->getSelection() || xournal->getControl()->getTextEditor()) {
        return false;
    }
    const auto device = event.deviceClass;
    if ((device == INPUT_DEVICE_MOUSE && isBlocked[InputContext::DeviceType::MOUSE]) ||
        (device == INPUT_DEVICE_PEN && isBlocked[InputContext::DeviceType::STYLUS]) ||
        (device == INPUT_DEVICE_TOUCHSCREEN && isBlocked[InputContext::DeviceType::TOUCHSCREEN])) {
        return false;  // don't intervene
    }

    switch (device) {
        case INPUT_DEVICE_KEYBOARD:
            return false;  // Handled via GtkEventControllerKey in InputContext
        case INPUT_DEVICE_TOUCHSCREEN:
            return this->handleTouchscreen(event);
        case INPUT_DEVICE_PEN:
        case INPUT_DEVICE_MOUSE:
        case INPUT_DEVICE_MOUSE_KEYBOARD_COMBO:
            return (!isBlocked[InputContext::DeviceType::MOUSE] && this->handlePointer(event));
        default:
            g_warning("Device class %d not handled by geometry tool", event.deviceClass);
            return false;
    }
}

auto GeometryToolInputHandler::handleTouchscreen(InputEvent const& event) -> bool {
    // Don't handle more than 2 inputs
    if (this->primarySequence && this->primarySequence != event.sequence && this->secondarySequence &&
        this->secondarySequence != event.sequence) {
        return false;
    }
    if (event.type == BUTTON_PRESS_EVENT) {
        // Start scrolling when a sequence starts and we currently have none other
        if (this->primarySequence == nullptr && this->secondarySequence == nullptr) {
            const xoj::util::Point<double> coords = getCoords(event);

            if (controller->isInsideGeometryTool(coords.x, coords.y, 0)) {
                // Set sequence data
                this->primarySequence = event.sequence;
                sequenceStart(event);
                return true;
            } else {
                return false;
            }

        }
        // Start zooming as soon as we have two sequences.
        else if (this->primarySequence && this->primarySequence != event.sequence &&
                 this->secondarySequence == nullptr) {
            this->secondarySequence = event.sequence;

            // Set sequence data
            sequenceStart(event);
            rotateAndZoomStart();
            return true;
        }
    }

    if (event.type == MOTION_EVENT && this->primarySequence) {
        if (this->secondarySequence) {
            rotateAndZoomMotion(event);
            return true;
        } else if (event.sequence == this->primarySequence) {
            scrollMotion(event);
            return true;
        }
        return false;
    }

    if (event.type == BUTTON_RELEASE_EVENT) {
        if (event.sequence == this->primarySequence) {
            // If secondarySequence is nullptr, this sets primarySequence
            // to nullptr. If it isn't, then it is now the primary sequence!
            this->primarySequence = std::exchange(this->secondarySequence, nullptr);
            this->priLastPageRel = this->secLastPageRel;
            return true;
        } else if (event.sequence == this->secondarySequence) {
            this->secondarySequence = nullptr;
            return true;
        }
    }
    return false;
}


template <int ANGLE_IN_DECIDEGREES>
static void rotate(GeometryToolController* ctrl) {
    ctrl->rotate(ANGLE_IN_DECIDEGREES * M_PI / 1800.);
}

template <bool DOWN, int AMOUNT>
static void scale(GeometryToolController* ctrl) {
    ctrl->scale(DOWN ? 100. / AMOUNT : AMOUNT / 100.);
}

static std::function<void(GeometryToolController*)> getMoveCb(bool alongAxis, double dx, double dy) {
    if (alongAxis) {
        return [dx, dy](GeometryToolController* ctrl) {
            double angle = ctrl->getGeometryTool()->getRotation();
            const double c = std::cos(angle);
            const double s = std::sin(angle);
            ctrl->translate({c * dx - s * dy, s * dx + c * dy});
        };
    } else {
        return [dx, dy](GeometryToolController* ctrl) { ctrl->translate({dx, dy}); };
    }
}

auto GeometryToolInputHandler::keyPressed(KeyEvent const& event) -> bool {
    // clang-format off
    static const KeyBindingsGroup<GeometryToolController> geometryKeyBindings({
        // Move
        {{NONE,        GDK_KEY_Left},  getMoveCb(false,       -MOVE_AMOUNT,                  0)},
        {{ALT,         GDK_KEY_Left},  getMoveCb(false, -MOVE_AMOUNT_SMALL,                  0)},
        {{SHIFT,       GDK_KEY_Left},  getMoveCb(true,        -MOVE_AMOUNT,                  0)},
        {{ALT & SHIFT, GDK_KEY_Left},  getMoveCb(true,  -MOVE_AMOUNT_SMALL,                  0)},
        {{NONE,        GDK_KEY_Right}, getMoveCb(false,        MOVE_AMOUNT,                  0)},
        {{ALT,         GDK_KEY_Right}, getMoveCb(false,  MOVE_AMOUNT_SMALL,                  0)},
        {{SHIFT,       GDK_KEY_Right}, getMoveCb(true,         MOVE_AMOUNT,                  0)},
        {{ALT & SHIFT, GDK_KEY_Right}, getMoveCb(true,   MOVE_AMOUNT_SMALL,                  0)},
        {{NONE,        GDK_KEY_Up},    getMoveCb(false,                  0,       -MOVE_AMOUNT)},
        {{ALT,         GDK_KEY_Up},    getMoveCb(false,                  0, -MOVE_AMOUNT_SMALL)},
        {{SHIFT,       GDK_KEY_Up},    getMoveCb(true,                   0,       -MOVE_AMOUNT)},
        {{ALT & SHIFT, GDK_KEY_Up},    getMoveCb(true,                   0, -MOVE_AMOUNT_SMALL)},
        {{NONE,        GDK_KEY_Down},  getMoveCb(false,                  0,        MOVE_AMOUNT)},
        {{ALT,         GDK_KEY_Down},  getMoveCb(false,                  0,  MOVE_AMOUNT_SMALL)},
        {{SHIFT,       GDK_KEY_Down},  getMoveCb(true,                   0,        MOVE_AMOUNT)},
        {{ALT & SHIFT, GDK_KEY_Down},  getMoveCb(true,                   0,  MOVE_AMOUNT_SMALL)},
        // Rotate
        {{NONE,        GDK_KEY_R},     rotate<       ROTATE_AMOUNT>},
        {{ALT,         GDK_KEY_R},     rotate< ROTATE_AMOUNT_SMALL>},
        {{NONE,        GDK_KEY_r},     rotate<      -ROTATE_AMOUNT>},
        {{ALT,         GDK_KEY_r},     rotate<-ROTATE_AMOUNT_SMALL>},
        // Scale
        {{NONE,        GDK_KEY_S},     scale<true,        SCALE_AMOUNT>},
        {{ALT,         GDK_KEY_S},     scale<true,  SCALE_AMOUNT_SMALL>},
        {{NONE,        GDK_KEY_s},     scale<false,       SCALE_AMOUNT>},
        {{ALT,         GDK_KEY_s},     scale<false, SCALE_AMOUNT_SMALL>},
        // Other
        {{NONE,        GDK_KEY_m},     KeyBindingsUtil::wrap<&GeometryToolController::markOrigin>}
    });
    // clang-format on
    return geometryKeyBindings.processEvent(controller, event);
}

void GeometryToolInputHandler::sequenceStart(InputEvent const& event) {
    if (event.sequence == this->primarySequence) {
        this->priLastPageRel = this->getCoords(event);
    } else {
        this->secLastPageRel = this->getCoords(event);
    }
    const auto page = controller->getPage();

    const Layer* layer = page->getSelectedLayer();
    this->lines.clear();
    Document* doc = xournal->getDocument();
    doc->lock();
    // Performance improvement might be obtained by avoiding filtering all elements each
    // time a finger has been put onto the screen
    for (const auto& e: layer->getElementsView()) {
        if (e->getType() == ELEMENT_STROKE) {
            auto* s = dynamic_cast<const Stroke*>(e);
            if (s->getPointCount() == 2) {
                lines.push_back(s);
            }
        }
    }
    doc->unlock();
}

void GeometryToolInputHandler::scrollMotion(InputEvent const& event) {
    // Will only be called if there is a single sequence (rotation/zooming handles two sequences)
    auto offset = [&]() {
        xoj::util::Point<double> coords = this->getCoords(event);
        if (event.sequence == this->primarySequence) {
            const xoj::util::Point<double> offset = coords - this->priLastPageRel;
            this->priLastPageRel = coords;
            return offset;
        } else {
            const xoj::util::Point<double> offset = coords - this->secLastPageRel;
            this->secLastPageRel = coords;
            return offset;
        }
    }();
    const auto& origin = controller->getGeometryTool()->getOrigin();
    const auto pos = Point(origin.x + offset.x, origin.y + offset.y);
    double minDist = SNAPPING_DISTANCE_TOLERANCE;
    double diffAngle{NAN};
    for (const auto& l: lines) {
        const Point first = l->getPoint(0);
        const Point second = l->getPoint(1);
        const double dist = Snapping::distanceLine(pos, first, second);
        const double angleLine = std::atan2(second.y - first.y, second.x - first.x);
        const double diff = std::remainder(angleLine - controller->getGeometryTool()->getRotation(), M_PI_2);
        if (dist < minDist && std::abs(diff) <= SNAPPING_ROTATION_TOLERANCE) {
            minDist = dist;
            diffAngle = diff;
        }
    }
    if (!std::isnan(diffAngle)) {
        controller->rotate(diffAngle, xoj::util::Point<double>(pos.x, pos.y));
    }
    controller->translate(offset);
}

void GeometryToolInputHandler::rotateAndZoomStart() {
    this->startZoomDistance = std::max(this->priLastPageRel.distance(this->secLastPageRel), ZOOM_DISTANCE_MIN);

    // Whether we can ignore the zoom portion of the gesture (e.g. distance between touch points
    // hasn't changed enough or zoom gestures are disabled).
    this->canBlockZoom = true;

    this->lastZoomScrollCenter = (this->priLastPageRel + this->secLastPageRel) / 2.0;
    const xoj::util::Point<double> shift = this->secLastPageRel - this->priLastPageRel;
    this->lastAngle = atan2(shift.y, shift.x);
    this->lastDist = this->startZoomDistance;
}

void GeometryToolInputHandler::rotateAndZoomMotion(InputEvent const& event) {
    if (event.sequence == this->primarySequence) {
        this->priLastPageRel = this->getCoords(event);
    } else {
        this->secLastPageRel = this->getCoords(event);
    }

    const double dist = std::max(this->priLastPageRel.distance(this->secLastPageRel), ZOOM_DISTANCE_MIN);

    const double zoomTriggerThreshold = xournal->getControl()->getSettings()->getTouchZoomStartThreshold();
    const double zoomChangePercentage = std::abs(dist - startZoomDistance) / startZoomDistance * 100;

    // Has the touch points moved far enough to trigger a zoom and are zoom gestures enabled?
    const bool zoomGesturesEnabled = xournal->getControl()->getSettings()->isZoomGesturesEnabled();
    if (zoomChangePercentage >= zoomTriggerThreshold && zoomGesturesEnabled) {
        this->canBlockZoom = false;
    }

    const xoj::util::Point<double> center = (this->priLastPageRel + this->secLastPageRel) / 2;
    const xoj::util::Point<double> shift = this->secLastPageRel - this->priLastPageRel;
    const double angle = atan2(shift.y, shift.x);

    const xoj::util::Point<double> offset = center - lastZoomScrollCenter;
    controller->translate(offset);
    const double angleIncrease = angle - lastAngle;
    const xoj::util::Point<double> centerRel = (this->priLastPageRel + this->secLastPageRel) / 2.0;
    if (controller->isInsideGeometryTool(secLastPageRel.x, secLastPageRel.y, 0.0)) {
        controller->rotate(angleIncrease, centerRel);
    }  // allow moving without accidental rotation

    if (!canBlockZoom) {
        controller->scale(dist / lastDist, centerRel);
    }

    this->lastZoomScrollCenter = center;
    this->lastAngle = angle;
    this->lastDist = dist;
}

auto GeometryToolInputHandler::getCoords(InputEvent const& event) -> xoj::util::Point<double> {
    const double zoom = xournal->getZoom();
    const auto view = controller->getView();
    return (event.relative - xoj::util::Point<double>(view->getX(), view->getY())) / zoom;
}

void GeometryToolInputHandler::blockDevice(InputContext::DeviceType deviceType) { isBlocked[deviceType] = true; }

void GeometryToolInputHandler::unblockDevice(InputContext::DeviceType deviceType) { isBlocked[deviceType] = false; }
