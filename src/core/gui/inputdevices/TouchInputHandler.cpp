//
// Created by ulrich on 08.04.19.
//

#include "TouchInputHandler.h"

#include <cmath>  // for abs

#include "control/Control.h"                        // for Control
#include "control/settings/Settings.h"              // for Settings
#include "control/zoom/ZoomControl.h"               // for ZoomControl
#include "gui/Layout.h"                             // for Layout
#include "gui/MainWindow.h"                         // for MainWindow
#include "gui/XournalView.h"                        // for XournalView
#include "gui/inputdevices/AbstractInputHandler.h"  // for AbstractInputHandler
#include "gui/inputdevices/InputEvents.h"           // for InputEvent, BUTTO...

#include "InputContext.h"  // for InputContext

TouchInputHandler::TouchInputHandler(InputContext* inputContext): AbstractInputHandler(inputContext) {}

auto TouchInputHandler::handleImpl(InputEvent const& event) -> bool {
    if (!event.sequence) {
        // On x11, a GDK_MOTION_EVENT with sequence == nullptr is emitted before TOUCH_BEGIN: Ignore it
        // In general, every valid touch event must have a sequence
        return false;
    }
    bool zoomGesturesEnabled = inputContext->getSettings()->isZoomGesturesEnabled();

    if (event.type == BUTTON_PRESS_EVENT) {
        if (invalidActive.find(event.sequence) != invalidActive.end()) {
            g_warning("Missed touch end/cancel event. Resetting touch input handler.");
            invalidActive.clear();
        }
        if (invalidActive.empty() && event.sequence != primarySequence && event.sequence != secondarySequence) {
            // All touches are previously valid and we did not miss an end/cancel event for the current touch
            if (primarySequence && secondarySequence) {
                // All touches become invalid
                g_debug("All touches become invalid");
                invalidActive.insert({primarySequence, secondarySequence, event.sequence});
                primarySequence = secondarySequence = nullptr;
            } else {
                if (primarySequence) {
                    secondarySequence = event.sequence;
                } else {
                    primarySequence = event.sequence;
                }
                sequenceStart(event);
                if (secondarySequence) {
                    startZoomReady = true;
                }
            }
        } else {
            invalidActive.insert(event.sequence);
            g_debug("Add touch as invalid. %zu touches are invalid now.", invalidActive.size());
        }
        return true;
    }

    if (event.type == MOTION_EVENT) {
        if (primarySequence == event.sequence && !secondarySequence) {
            scrollMotion(event);
            return true;
        } else if (event.sequence && zoomGesturesEnabled &&
                   (primarySequence == event.sequence || secondarySequence == event.sequence)) {
            xoj_assert(primarySequence);
            if (startZoomReady) {
                if (this->primarySequence == event.sequence) {
                    sequenceStart(event);
                    zoomStart();
                }
            } else {
                zoomMotion(event);
            }
            return true;
        }
    }

    if (event.type == BUTTON_RELEASE_EVENT) {
        if (zooming && primarySequence && secondarySequence &&
            (event.sequence == primarySequence || event.sequence == secondarySequence)) {
            zoomEnd();
        }

        if (event.sequence == primarySequence) {
            // If secondarySequence is nullptr, this sets primarySequence
            // to nullptr. If it isn't, then it is now the primary sequence!
            primarySequence = std::exchange(secondarySequence, nullptr);

            this->priLastAbs = this->secLastAbs;
            this->priLastRel = this->secLastRel;
        } else if (event.sequence == secondarySequence) {
            secondarySequence = nullptr;
        } else {
            invalidActive.erase(event.sequence);
            g_debug("Removing sequence from invalid list, %zu inputs remain invalid.", invalidActive.size());
        }
        return true;
    }

    return false;
}

void TouchInputHandler::sequenceStart(InputEvent const& event) {
    if (event.sequence == this->primarySequence) {
        this->priLastAbs = event.absolute;
        this->priLastRel = event.relative;
    } else {
        this->secLastAbs = event.absolute;
        this->secLastRel = event.relative;
    }
}

void TouchInputHandler::scrollMotion(InputEvent const& event) {
    // Will only be called if there is a single sequence (zooming handles two sequences)
    auto offset = [&]() {
        if (event.sequence == this->primarySequence) {
            auto offset = event.absolute - this->priLastAbs;
            this->priLastAbs = event.absolute;
            return offset;
        } else {
            auto offset = event.absolute - this->secLastAbs;
            this->secLastAbs = event.absolute;
            return offset;
        }
    }();

    auto* layout = inputContext->getView()->getControl()->getWindow()->getLayout();

    layout->scrollRelative(-offset.x, -offset.y);
}

void TouchInputHandler::zoomStart() {
    this->zooming = true;
    this->startZoomDistance = this->priLastAbs.distance(this->secLastAbs);

    if (this->startZoomDistance == 0.0) {
        this->startZoomDistance = 0.01;
    }

    // Whether we can ignore the zoom portion of the gesture (e.g. distance between touch points
    // hasn't changed enough).
    this->canBlockZoom = true;

    ZoomControl* zoomControl = this->inputContext->getView()->getControl()->getZoomControl();

    // Disable zoom fit as we are zooming currently
    // TODO(fabian): this should happen internally!!!
    if (zoomControl->isZoomFitMode()) {
        zoomControl->setZoomFitMode(false);
    }

    // use screen pixel coordinates for the zoom center
    // as relative coordinates depend on the changing zoom level
    auto center = (this->priLastAbs + this->secLastAbs) / 2.0;
    this->lastZoomScrollCenter = center;

    // translate absolute window coordinates to the widget-local coordinates
    const auto* mainWindow = inputContext->getView()->getControl()->getWindow();
    const auto translation = mainWindow->getNegativeXournalWidgetPos();
    center += translation;

    zoomControl->startZoomSequence(center);

    this->startZoomReady = false;
}

void TouchInputHandler::zoomMotion(InputEvent const& event) {
    if (event.sequence == this->primarySequence) {
        this->priLastAbs = event.absolute;
    } else {
        this->secLastAbs = event.absolute;
    }

    double distance = this->priLastAbs.distance(this->secLastAbs);
    double zoom = distance / this->startZoomDistance;

    double zoomTriggerThreshold = inputContext->getSettings()->getTouchZoomStartThreshold();
    double zoomChangePercentage = std::abs(distance - startZoomDistance) / startZoomDistance * 100;

    // Has the touch points moved far enough to trigger a zoom?
    if (this->canBlockZoom && zoomChangePercentage < zoomTriggerThreshold) {
        zoom = 1.0;
    } else {
        // Touches have moved far enough from their initial location that we
        // no longer prevent touchscreen zooming.
        this->canBlockZoom = false;
    }

    ZoomControl* zoomControl = this->inputContext->getView()->getControl()->getZoomControl();
    const auto center = (this->priLastAbs + this->secLastAbs) / 2;
    zoomControl->zoomSequenceChange(zoom, true, center - lastZoomScrollCenter);
    lastZoomScrollCenter = center;
}

void TouchInputHandler::zoomEnd() {
    this->zooming = false;
    ZoomControl* zoomControl = this->inputContext->getView()->getControl()->getZoomControl();
    zoomControl->endZoomSequence();
}

void TouchInputHandler::onBlock() {
    if (this->zooming) {
        zoomEnd();
    }
}

void TouchInputHandler::onUnblock() {
    this->invalidActive.clear();
    this->primarySequence = nullptr;
    this->secondarySequence = nullptr;

    this->startZoomDistance = 0.0;
    this->lastZoomScrollCenter = {};

    priLastAbs = {-1.0, -1.0};
    secLastAbs = {-1.0, -1.0};
    priLastRel = {-1.0, -1.0};
    secLastRel = {-1.0, -1.0};
}
