#include "GeometryToolInputHandler.h"

#include <algorithm>  // for max
#include <cmath>      // for atan2, abs, cos, remainder
#include <memory>     // for __shared_ptr_access, shar...

#include <gdk/gdkkeysyms.h>  // for GDK_KEY_Down, GDK_KEY_Left
#include <glib.h>            // for g_warning

#include "control/Control.h"                 // for Control
#include "control/GeometryToolController.h"  // for GeometryToolController
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
constexpr double ROTATE_AMOUNT = M_PI * 5.0 / 180.0;
constexpr double ROTATE_AMOUNT_SMALL = M_PI * 0.2 / 180.0;
constexpr double SCALE_AMOUNT = 1.1;
constexpr double SCALE_AMOUNT_SMALL = 1.01;
constexpr double SNAPPING_DISTANCE_TOLERANCE = 5.0;                 // pt
constexpr double SNAPPING_ROTATION_TOLERANCE = 3.0 * M_PI / 180.0;  // rad
constexpr double ZOOM_DISTANCE_MIN = 0.01;

GeometryToolInputHandler::GeometryToolInputHandler(XournalView* xournal, GeometryToolController* controller, double h,
                                                   double tx, double ty):
        xournal(xournal), controller(controller), height(h), translationX(tx), translationY(ty) {}

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
            return this->handleKeyboard(event);
        case INPUT_DEVICE_TOUCHSCREEN:
            return this->handleTouchscreen(event);
        case INPUT_DEVICE_PEN:
        case INPUT_DEVICE_MOUSE:
            return this->handlePointer(event);
        case INPUT_DEVICE_MOUSE_KEYBOARD_COMBO:
            return this->handlePointer(event) || this->handleKeyboard(event);
        default:
            g_warning("Device class %d not handled by geometry tool", event.deviceClass);
            return false;
    }
}

void GeometryToolInputHandler::on(UpdateValuesRequest, double h, double rot, double tx, double ty) {
    height = h;
    rotation = rot;
    translationX = tx;
    translationY = ty;
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
            const utl::Point<double> coords = getCoords(event);

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

auto GeometryToolInputHandler::handleKeyboard(InputEvent const& event) -> bool {
    GdkEvent* gdkEvent = event.sourceEvent;
    if (gdk_event_get_event_type(gdkEvent) == GDK_KEY_PRESS) {
        double xdir = 0;
        double ydir = 0;
        double angle = 0.0;
        double scale = 1.0;
        guint key;
        if (!gdk_event_get_keyval(gdkEvent, &key)) {
            return false;
        }
        switch (key) {
            case GDK_KEY_Left:
                xdir = -1;
                break;
            case GDK_KEY_Up:
                ydir = -1;
                break;
            case GDK_KEY_Right:
                xdir = 1;
                break;
            case GDK_KEY_Down:
                ydir = 1;
                break;
            case GDK_KEY_r:
            case GDK_KEY_R: {  // r like "rotate"
                angle = (event.state & GDK_MOD1_MASK) ? ROTATE_AMOUNT_SMALL : ROTATE_AMOUNT;
                angle = (event.state & GDK_SHIFT_MASK) ? angle : -angle;
                break;
            }
            case GDK_KEY_s:
            case GDK_KEY_S: {
                scale = (event.state & GDK_MOD1_MASK) ? SCALE_AMOUNT_SMALL : SCALE_AMOUNT;
                scale = (event.state & GDK_SHIFT_MASK) ? 1.0 / scale : scale;
                const double h = height * scale;
                if (h > getMaxHeight() || h < getMinHeight()) {
                    scale = 1.0;
                }
                break;
            }
            case GDK_KEY_m: {
                controller->markPoint(translationX, translationY);
                return true;
            }
        }

        if (xdir != 0 || ydir != 0) {
            const double c = std::cos(rotation);
            const double s = std::sin(rotation);
            double xshift{0.0};
            double yshift{0.0};
            const double amount = (event.state & GDK_MOD1_MASK) ? MOVE_AMOUNT_SMALL : MOVE_AMOUNT;
            if (event.state & GDK_SHIFT_MASK) {
                xshift = amount * (c * xdir - s * ydir);
                yshift = amount * (s * xdir + c * ydir);
            } else {
                xshift = amount * xdir;
                yshift = amount * ydir;
            }
            controller->translate(xshift, yshift);
            return true;
        }

        auto p = utl::Point(translationX, translationY);
        if (angle != 0) {
            controller->rotate(angle, p.x, p.y);
            return true;
        }
        if (scale != 1.0) {
            controller->scale(scale, p.x, p.y);
            return true;
        }
    }
    return false;
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
    for (const auto& e: layer->getElements()) {
        if (e->getType() == ELEMENT_STROKE) {
            Stroke* s = dynamic_cast<Stroke*>(e);
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
        utl::Point<double> coords = this->getCoords(event);
        if (event.sequence == this->primarySequence) {
            const utl::Point<double> offset = coords - this->priLastPageRel;
            this->priLastPageRel = coords;
            return offset;
        } else {
            const utl::Point<double> offset = coords - this->secLastPageRel;
            this->secLastPageRel = coords;
            return offset;
        }
    }();
    const auto pos = Point(translationX + offset.x, translationY + offset.y);
    double minDist = SNAPPING_DISTANCE_TOLERANCE;
    double diffAngle{NAN};
    for (const auto& l: lines) {
        const Point first = l->getPoint(0);
        const Point second = l->getPoint(1);
        const double dist = Snapping::distanceLine(pos, first, second);
        const double angleLine = std::atan2(second.y - first.y, second.x - first.x);
        const double diff = std::remainder(angleLine - rotation, M_PI_2);
        if (dist < minDist && std::abs(diff) <= SNAPPING_ROTATION_TOLERANCE) {
            minDist = dist;
            diffAngle = diff;
        }
    }
    if (!std::isnan(diffAngle)) {
        controller->rotate(diffAngle, pos.x, pos.y);
    }
    controller->translate(offset.x, offset.y);
}

void GeometryToolInputHandler::rotateAndZoomStart() {
    this->startZoomDistance = std::max(this->priLastPageRel.distance(this->secLastPageRel), ZOOM_DISTANCE_MIN);

    // Whether we can ignore the zoom portion of the gesture (e.g. distance between touch points
    // hasn't changed enough or zoom gestures are disabled).
    this->canBlockZoom = true;

    this->lastZoomScrollCenter = (this->priLastPageRel + this->secLastPageRel) / 2.0;
    const utl::Point<double> shift = this->secLastPageRel - this->priLastPageRel;
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

    const utl::Point<double> center = (this->priLastPageRel + this->secLastPageRel) / 2;
    const utl::Point<double> shift = this->secLastPageRel - this->priLastPageRel;
    const double angle = atan2(shift.y, shift.x);

    const utl::Point<double> offset = center - lastZoomScrollCenter;
    controller->translate(offset.x, offset.y);
    const double angleIncrease = angle - lastAngle;
    const utl::Point<double> centerRel = (this->priLastPageRel + this->secLastPageRel) / 2.0;
    if (controller->isInsideGeometryTool(secLastPageRel.x, secLastPageRel.y, 0.0)) {
        controller->rotate(angleIncrease, centerRel.x, centerRel.y);
    }  // allow moving without accidental rotation
    const double scaleFactor = dist / lastDist;
    const double h = height * scaleFactor;

    if (!canBlockZoom && h <= getMaxHeight() && h >= getMinHeight()) {
        controller->scale(scaleFactor, centerRel.x, centerRel.y);
    }

    this->lastZoomScrollCenter = center;
    this->lastAngle = angle;
    this->lastDist = dist;
}

auto GeometryToolInputHandler::getCoords(InputEvent const& event) -> utl::Point<double> {
    const double zoom = xournal->getZoom();
    const auto view = controller->getView();
    const double posX = event.relativeX - static_cast<double>(view->getX());
    const double posY = event.relativeY - static_cast<double>(view->getY());
    return utl::Point<double>(posX / zoom, posY / zoom);
}

void GeometryToolInputHandler::blockDevice(InputContext::DeviceType deviceType) { isBlocked[deviceType] = true; }

void GeometryToolInputHandler::unblockDevice(InputContext::DeviceType deviceType) { isBlocked[deviceType] = false; }
