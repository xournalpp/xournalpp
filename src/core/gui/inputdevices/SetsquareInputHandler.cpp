#include "SetsquareInputHandler.h"

#include <algorithm>  // for max
#include <cmath>      // for atan2, abs, cos, remainder
#include <memory>     // for __shared_ptr_access, shar...

#include <gdk/gdkkeysyms.h>  // for GDK_KEY_Down, GDK_KEY_Left
#include <glib.h>            // for g_warning

#include "control/Control.h"                // for Control
#include "control/ToolEnums.h"              // for TOOL_HAND, TOOL_HIGHLIGHTER
#include "control/ToolHandler.h"            // for ToolHandler
#include "control/settings/Settings.h"      // for Settings
#include "gui/PageView.h"                   // for XojPageView
#include "gui/XournalView.h"                // for XournalView
#include "gui/inputdevices/InputContext.h"  // for InputContext::DeviceType
#include "gui/inputdevices/InputEvents.h"   // for InputEvent, BUTTON_PRESS_...
#include "gui/widgets/XournalWidget.h"      // for GtkXournal
#include "model/Element.h"                  // for Element, ELEMENT_STROKE
#include "model/Layer.h"                    // for Layer
#include "model/Point.h"                    // for Point
#include "model/Setsquare.h"                // for HALF_CM
#include "model/Snapping.h"                 // for distanceLine
#include "model/Stroke.h"                   // for Stroke
#include "model/XojPage.h"                  // for XojPage
#include "view/SetsquareView.h"             // for SetsquareView, HYPOTENUSE

constexpr double MOVE_AMOUNT = HALF_CM / 2.0;
constexpr double MOVE_AMOUNT_SMALL = HALF_CM / 20.0;
constexpr double ROTATE_AMOUNT = M_PI * 5.0 / 180.0;
constexpr double ROTATE_AMOUNT_SMALL = M_PI * 0.2 / 180.0;
constexpr double SCALE_AMOUNT = 1.1;
constexpr double SCALE_AMOUNT_SMALL = 1.01;
constexpr double MIN_HEIGHT = 4.5;
constexpr double MAX_HEIGHT = 15.0;
constexpr double SNAPPING_DISTANCE_TOLERANCE = 5.0;                 // pt
constexpr double SNAPPING_ROTATION_TOLERANCE = 3.0 * M_PI / 180.0;  // rad
constexpr double ZOOM_DISTANCE_MIN = 0.01;

SetsquareInputHandler::SetsquareInputHandler(InputContext* inputContext): inputContext(inputContext) {}

auto SetsquareInputHandler::handle(InputEvent const& event) -> bool {
    if (!inputContext->getView()->getSetsquareView() || inputContext->getXournal()->selection ||
        inputContext->getView()->getControl()->getTextEditor()) {
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
            g_warning("Device class %d not handled by setsquare", event.deviceClass);
            return false;
    }
}

auto SetsquareInputHandler::handleTouchscreen(InputEvent const& event) -> bool {
    const auto zoomGesturesEnabled = inputContext->getSettings()->isZoomGesturesEnabled();
    // Don't handle more then 2 inputs
    if (this->primarySequence && this->primarySequence != event.sequence && this->secondarySequence &&
        this->secondarySequence != event.sequence) {
        return false;
    }
    if (event.type == BUTTON_PRESS_EVENT) {
        // Start scrolling when a sequence starts and we currently have none other
        if (this->primarySequence == nullptr && this->secondarySequence == nullptr) {
            const auto view = inputContext->getView();
            const auto setsquareView = view->getSetsquareView();
            auto coords = getCoords(event);

            if (setsquareView->isInsideSetsquare(coords.x, coords.y, 0)) {
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

            // Even if zoom gestures are disabled,
            // this is still the start of a sequence. Just
            // don't start zooming.
            if (zoomGesturesEnabled) {
                zoomStart();
                return true;
            }
        }
    }

    if (event.type == MOTION_EVENT && this->primarySequence) {
        if (this->primarySequence && this->secondarySequence && zoomGesturesEnabled) {
            zoomMotion(event);
            return true;
        } else if (event.sequence == this->primarySequence) {
            scrollMotion(event);
            return true;
        } else if (this->primarySequence && this->secondarySequence) {
            sequenceStart(event);
            return true;
        }
    }

    if (event.type == BUTTON_RELEASE_EVENT) {
        if (event.sequence == this->primarySequence) {
            // If secondarySequence is nullptr, this sets primarySequence
            // to nullptr. If it isn't, then it is now the primary sequence!
            this->primarySequence = this->secondarySequence;
            this->secondarySequence = nullptr;

            this->priLastAbs = this->secLastAbs;
            this->priLastRel = this->secLastRel;
        } else {
            this->secondarySequence = nullptr;
        }
    }
    return false;
}

auto SetsquareInputHandler::handleKeyboard(InputEvent const& event) -> bool {
    GdkEvent* gdkEvent = event.sourceEvent;
    if (gdkEvent->type == GDK_KEY_PRESS) {
        auto keyEvent = reinterpret_cast<GdkEventKey*>(gdkEvent);

        double xdir = 0;
        double ydir = 0;
        double angle = 0.0;
        double scale = 1.0;

        const auto view = inputContext->getView();
        const auto setsquareView = view->getSetsquareView();

        switch (keyEvent->keyval) {
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
                const auto height = setsquareView->getHeight() * scale;
                if (height > MAX_HEIGHT || height < MIN_HEIGHT) {
                    scale = 1.0;
                }
                break;
            }
        }

        if (xdir != 0 || ydir != 0) {
            const auto c = std::cos(setsquareView->getRotation());
            const auto s = std::sin(setsquareView->getRotation());
            double xshift{0.0};
            double yshift{0.0};
            const auto amount = (event.state & GDK_MOD1_MASK) ? MOVE_AMOUNT_SMALL : MOVE_AMOUNT;
            if (event.state & GDK_SHIFT_MASK) {
                xshift = amount * (c * xdir - s * ydir);
                yshift = amount * (s * xdir + c * ydir);
            } else {
                xshift = amount * xdir;
                yshift = amount * ydir;
            }
            setsquareView->move(xshift, yshift);
            view->repaintSetsquare();
            return true;
        }

        auto p = utl::Point(setsquareView->getTranslationX(), setsquareView->getTranslationY());
        if (angle != 0) {
            setsquareView->rotate(angle, p.x, p.y);
            view->repaintSetsquare();
            return true;
        }
        if (scale != 1.0) {
            setsquareView->scale(scale, p.x, p.y);
            view->repaintSetsquare();
            return true;
        }
    }
    return false;
}

auto SetsquareInputHandler::handlePointer(InputEvent const& event) -> bool {
    const auto view = inputContext->getView();
    const auto setsquareView = view->getSetsquareView();
    const auto coords = getCoords(event);

    const auto toolHandler = view->getControl()->getToolHandler();
    switch (toolHandler->getToolType()) {
        case TOOL_HIGHLIGHTER:
        case TOOL_PEN:
            if (event.type == BUTTON_PRESS_EVENT) {
                if (setsquareView->isInsideSetsquare(coords.x, coords.y, 0) &&
                    setsquareView->posRelToSide(HYPOTENUSE, coords.x, coords.y).y >= -0.5) {
                    // initialize range
                    const auto proj = setsquareView->posRelToSide(HYPOTENUSE, coords.x, coords.y).x;
                    setsquareView->createStroke(proj);
                    return true;
                } else if (setsquareView->isInsideSetsquare(coords.x, coords.y, 0) &&
                           (setsquareView->posRelToSide(LEFT_LEG, coords.x, coords.y).y >= -0.5 ||
                            setsquareView->posRelToSide(RIGHT_LEG, coords.x, coords.y).y >= -0.5)) {

                    // initialize point
                    setsquareView->createRadius(coords.x, coords.y);
                    return true;
                } else {
                    return false;
                }

            } else if (event.type == MOTION_EVENT) {
                // update range and paint
                if (setsquareView->existsStroke()) {
                    const auto proj = setsquareView->posRelToSide(HYPOTENUSE, coords.x, coords.y).x;
                    setsquareView->updateStroke(proj);
                    return true;
                } else if (setsquareView->existsRadius()) {
                    setsquareView->updateRadius(coords.x, coords.y);
                    return true;
                }
                return false;
            } else if (event.type == BUTTON_RELEASE_EVENT) {
                // add stroke to layer and reset
                if (setsquareView->existsStroke()) {
                    setsquareView->finalizeStroke();
                    return true;
                } else if (setsquareView->existsRadius()) {
                    setsquareView->finalizeRadius();
                    return true;
                }
            }
            return false;
        case TOOL_HAND:
            if (event.type == BUTTON_PRESS_EVENT) {
                if (!setsquareView->isInsideSetsquare(coords.x, coords.y, 0)) {
                    return false;
                } else {
                    sequenceStart(event);
                    this->handScrolling = true;
                    return true;
                }
            } else if (event.type == MOTION_EVENT && this->handScrolling) {
                scrollMotion(event);
                return true;
            } else if (event.type == BUTTON_RELEASE_EVENT && this->handScrolling) {
                this->handScrolling = false;
            }
        default:
            return false;
    }
}

void SetsquareInputHandler::sequenceStart(InputEvent const& event) {
    if (event.sequence == this->primarySequence) {
        this->priLastAbs = {event.absoluteX, event.absoluteY};
        this->priLastRel = {event.relativeX, event.relativeY};
    } else {
        this->secLastAbs = {event.absoluteX, event.absoluteY};
        this->secLastRel = {event.relativeX, event.relativeY};
    }
    const auto view = inputContext->getView();
    const auto setsquareView = view->getSetsquareView();
    const auto page = setsquareView->getView()->getPage();

    const auto l = page->getSelectedLayer();
    this->lines.clear();
    for (const auto& e: l->getElements()) {
        if (e->getType() == ELEMENT_STROKE) {
            auto s = dynamic_cast<Stroke*>(e);
            if (s->getPointCount() == 2) {
                lines.push_back(s);
            }
        }
    }
}

void SetsquareInputHandler::scrollMotion(InputEvent const& event) {
    // Will only be called if there is a single sequence (zooming handles two sequences)
    auto offset = [&]() {
        auto absolutePoint = utl::Point{event.absoluteX, event.absoluteY};
        if (event.sequence == this->primarySequence) {
            const auto offset = absolutePoint - this->priLastAbs;
            this->priLastAbs = absolutePoint;
            return offset;
        } else {
            const auto offset = absolutePoint - this->secLastAbs;
            this->secLastAbs = absolutePoint;
            return offset;
        }
    }();
    const auto view = inputContext->getView();
    const auto setsquareView = view->getSetsquareView();
    const auto zoom = view->getZoom();
    auto p = utl::Point(setsquareView->getTranslationX(), setsquareView->getTranslationY());
    p.x += offset.x / zoom;
    p.y += offset.y / zoom;
    const auto pos = Point(p.x, p.y);
    double minDist = SNAPPING_DISTANCE_TOLERANCE;
    // Point snappedPoint{};
    const auto angleSetsquare = setsquareView->getRotation();
    double diffAngle{NAN};
    for (const auto& l: lines) {
        auto first = l->getPoint(0);
        auto second = l->getPoint(1);
        auto dist = Snapping::distanceLine(pos, first, second);
        auto angleLine = std::atan2(second.y - first.y, second.x - first.x);
        auto diff = std::remainder(angleLine - angleSetsquare, M_PI_2);
        if (dist < minDist && std::abs(diff) <= SNAPPING_ROTATION_TOLERANCE) {
            minDist = dist;
            diffAngle = diff;
            // snappedPoint = Snapping::snapToLine(pos, first, second, SNAPPING_DISTANCE_TOLERANCE);
        }
    }
    if (!std::isnan(diffAngle)) {
        setsquareView->rotate(diffAngle, p.x, p.y);
        //    setsquareView->move(snappedPoint.x - p.x, snappedPoint.y - p.y);
    }
    setsquareView->move(offset.x / zoom, offset.y / zoom);
    view->repaintSetsquare();
}

void SetsquareInputHandler::zoomStart() {
    this->startZoomDistance = std::max(this->priLastAbs.distance(this->secLastAbs), ZOOM_DISTANCE_MIN);

    // Whether we can ignore the zoom portion of the gesture (e.g. distance between touch points
    // hasn't changed enough).
    this->canBlockZoom = true;

    this->lastZoomScrollCenter = (this->priLastAbs + this->secLastAbs) / 2.0;
    const auto shift = this->secLastAbs - this->priLastAbs;
    this->lastAngle = atan2(shift.y, shift.x);
    this->lastDist = this->startZoomDistance;
}

void SetsquareInputHandler::zoomMotion(InputEvent const& event) {
    if (event.sequence == this->primarySequence) {
        this->priLastAbs = {event.absoluteX, event.absoluteY};
        this->priLastRel = {event.relativeX, event.relativeY};
    } else {
        this->secLastAbs = {event.absoluteX, event.absoluteY};
        this->secLastRel = {event.relativeX, event.relativeY};
    }

    const double dist = std::max(this->priLastAbs.distance(this->secLastAbs), ZOOM_DISTANCE_MIN);

    const auto zoomTriggerThreshold = inputContext->getSettings()->getTouchZoomStartThreshold();
    const double zoomChangePercentage = std::abs(dist - startZoomDistance) / startZoomDistance * 100;

    // Has the touch points moved far enough to trigger a zoom?
    if (zoomChangePercentage >= zoomTriggerThreshold) {
        this->canBlockZoom = false;
    }

    const auto center = (this->priLastAbs + this->secLastAbs) / 2;
    const auto shift = this->secLastAbs - priLastAbs;
    const auto angle = atan2(shift.y, shift.x);

    const auto view = inputContext->getView();
    const auto setsquareView = view->getSetsquareView();
    const auto offset = center - lastZoomScrollCenter;
    const auto zoom = view->getZoom();
    setsquareView->move(offset.x / zoom, offset.y / zoom);
    const auto angleIncrease = angle - lastAngle;
    const auto centerRel = (this->priLastRel + this->secLastRel) / 2.0;
    const auto coords = getCoords(centerRel.x, centerRel.y);
    const auto secondary = getCoords(this->secLastRel.x, this->secLastRel.y);
    if (setsquareView->isInsideSetsquare(secondary.x, secondary.y, 0.0)) {
        setsquareView->rotate(angleIncrease, coords.x, coords.y);
    }  // allow moving without accidental rotation (snapping)0.01
    const auto scaleFactor = dist / lastDist;
    const auto height = setsquareView->getHeight() * scaleFactor;

    if (!canBlockZoom && height <= MAX_HEIGHT && height >= MIN_HEIGHT) {
        setsquareView->scale(scaleFactor, coords.x, coords.y);
    }
    view->repaintSetsquare();


    this->lastZoomScrollCenter = center;
    this->lastAngle = angle;
    this->lastDist = dist;
}

auto SetsquareInputHandler::getCoords(double xCoord, double yCoord) -> utl::Point<double> {
    const auto view = inputContext->getView();
    const auto setsquareView = view->getSetsquareView();
    const auto zoom = view->getZoom();
    const auto page = setsquareView->getView();
    const auto posX = xCoord - static_cast<double>(page->getX());
    const auto posY = yCoord - static_cast<double>(page->getY());
    return utl::Point<double>(posX / zoom, posY / zoom);
}

auto SetsquareInputHandler::getCoords(InputEvent const& event) -> utl::Point<double> {
    return getCoords(event.relativeX, event.relativeY);
}

void SetsquareInputHandler::blockDevice(InputContext::DeviceType deviceType) { isBlocked[deviceType] = true; }

void SetsquareInputHandler::unblockDevice(InputContext::DeviceType deviceType) { isBlocked[deviceType] = false; }
