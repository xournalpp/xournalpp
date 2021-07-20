//
// Created by ulrich on 08.04.19.
//

#include "TouchInputHandler.h"

#include <cmath>

#include "view/SetsquareView.h"

#include "InputContext.h"

TouchInputHandler::TouchInputHandler(InputContext* inputContext): AbstractInputHandler(inputContext) {}

auto TouchInputHandler::handleImpl(InputEvent const& event) -> bool {
    bool zoomGesturesEnabled = inputContext->getSettings()->isZoomGesturesEnabled();
    auto setsquareView = inputContext->getXournal()->setsquareView;
    bool setsquareGesture = setsquareView != nullptr;
    // Don't handle more then 2 inputs
    if (this->primarySequence && this->primarySequence != event.sequence && this->secondarySequence &&
        this->secondarySequence != event.sequence) {
        return false;
    }

    if (event.type == BUTTON_PRESS_EVENT) {
        // Start scrolling when a sequence starts and we currently have none other
        if (this->primarySequence == nullptr && this->secondarySequence == nullptr) {
            this->primarySequence = event.sequence;

            // Set sequence data
            sequenceStart(event, setsquareGesture);
        }
        // Start zooming as soon as we have two sequences.
        else if (this->primarySequence && this->primarySequence != event.sequence &&
                 this->secondarySequence == nullptr) {
            this->secondarySequence = event.sequence;

            // Set sequence data
            sequenceStart(event, setsquareGesture);

            // Even if zoom gestures are disabled,
            // this is still the start of a sequence. Just
            // don't start zooming.
            if (zoomGesturesEnabled) {
                zoomStart();
            }
        }
    }

    if (event.type == MOTION_EVENT && this->primarySequence) {
        if (this->primarySequence && this->secondarySequence && zoomGesturesEnabled) {
            zoomMotion(event);
        } else if (event.sequence == this->primarySequence) {
            scrollMotion(event);
        } else if (this->primarySequence && this->secondarySequence) {
            sequenceStart(event, setsquareGesture);
        }
    }

    if (event.type == BUTTON_RELEASE_EVENT) {
        // Only stop zooming if both sequences were active (we were scrolling)
        if (this->primarySequence != nullptr && this->secondarySequence != nullptr && zoomGesturesEnabled) {
            zoomEnd();
        }

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

void TouchInputHandler::sequenceStart(InputEvent const& event, bool setsquareGesture) {
    if (event.sequence == this->primarySequence) {
        this->priLastAbs = {event.absoluteX, event.absoluteY};
        this->priLastRel = {event.relativeX, event.relativeY};
    } else {
        this->secLastAbs = {event.absoluteX, event.absoluteY};
        this->secLastRel = {event.relativeX, event.relativeY};
    }
    this->isSetsquareGesture = setsquareGesture;
}

void TouchInputHandler::scrollMotion(InputEvent const& event) {
    // Will only be called if there is a single sequence (zooming handles two sequences)
    auto offset = [&]() {
        auto absolutePoint = utl::Point{event.absoluteX, event.absoluteY};
        if (event.sequence == this->primarySequence) {
            auto offset = absolutePoint - this->priLastAbs;
            this->priLastAbs = absolutePoint;
            return offset;
        } else {
            auto offset = absolutePoint - this->secLastAbs;
            this->secLastAbs = absolutePoint;
            return offset;
        }
    }();
    if (isSetsquareGesture) {
        SetsquareView* setsquareView = inputContext->getXournal()->setsquareView;
        setsquareView->move(offset.x, offset.y);
        auto view = setsquareView->getView();
        view->getXournal()->repaintSetsquare();
    } else {
        auto* layout = inputContext->getView()->getControl()->getWindow()->getLayout();
        layout->scrollRelative(-offset.x, -offset.y);
    }
}

void TouchInputHandler::zoomStart() {
    // Take horizontal and vertical padding of view into account when calculating the center of the gesture
    int vPadding = inputContext->getSettings()->getAddVerticalSpace() ?
                           inputContext->getSettings()->getAddVerticalSpaceAmount() :
                           0;
    int hPadding = inputContext->getSettings()->getAddHorizontalSpace() ?
                           inputContext->getSettings()->getAddHorizontalSpaceAmount() :
                           0;

    auto center = (this->priLastRel + this->secLastRel) / 2.0 - utl::Point<double>{double(hPadding), double(vPadding)};

    this->startZoomDistance = this->priLastAbs.distance(this->secLastAbs);

    if (this->startZoomDistance == 0.0) {
        this->startZoomDistance = 0.01;
    }

    // Whether we can ignore the zoom portion of the gesture (e.g. distance between touch points
    // hasn't changed enough).
    this->canBlockZoom = true;

    lastZoomScrollCenter = (this->priLastAbs + this->secLastAbs) / 2.0;
    auto v = this->secLastAbs - this->priLastAbs;
    lastAngle = atan2(v.y, v.x);
    lastDist = this->secLastAbs.distance(this->priLastAbs);

    if (isSetsquareGesture) {
    } else {

        ZoomControl* zoomControl = this->inputContext->getView()->getControl()->getZoomControl();

        // Disable zoom fit as we are zooming currently
        // TODO(fabian): this should happen internally!!!
        if (zoomControl->isZoomFitMode()) {
            zoomControl->setZoomFitMode(false);
        }

        auto* mainWindow = inputContext->getView()->getControl()->getWindow();

        // When not using touch drawing, we're using a different scrolling method.
        // This requires different centering.
        if (!mainWindow->getGtkTouchscreenScrollingEnabled()) {
            Rectangle zoomSequenceRectangle = zoomControl->getVisibleRect();
            center += utl::Point<double>{zoomSequenceRectangle.x, zoomSequenceRectangle.y};
        }

        zoomControl->startZoomSequence(center);
    }
}

void TouchInputHandler::zoomMotion(InputEvent const& event) {
    if (event.sequence == this->primarySequence) {
        this->priLastAbs = {event.absoluteX, event.absoluteY};
    } else {
        this->secLastAbs = {event.absoluteX, event.absoluteY};
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

    auto center = (this->priLastAbs + this->secLastAbs) / 2;
    auto v = this->secLastAbs - priLastAbs;
    auto angle = atan2(v.y, v.x);
    auto dist = this->secLastAbs.distance(priLastAbs);

    if (isSetsquareGesture) {
        SetsquareView* setsquareView = inputContext->getXournal()->setsquareView;
        auto offset = center - lastZoomScrollCenter;
        setsquareView->move(offset.x, offset.y);
        auto da = angle - lastAngle;
        setsquareView->rotate(da, center.x, center.y);
        auto f = dist / lastDist;
        setsquareView->scale(f);
        auto view = setsquareView->getView();
        view->getXournal()->repaintSetsquare();

    } else {
        ZoomControl* zoomControl = this->inputContext->getView()->getControl()->getZoomControl();
        zoomControl->zoomSequenceChange(zoom, true);

        auto lastScrollPosition = zoomControl->getScrollPositionAfterZoom();
        auto offset = lastScrollPosition - (center - lastZoomScrollCenter);

        zoomControl->setScrollPositionAfterZoom(offset);
    }

    lastZoomScrollCenter = center;
    lastAngle = angle;
    lastDist = dist;
}

void TouchInputHandler::zoomEnd() {
    ZoomControl* zoomControl = this->inputContext->getView()->getControl()->getZoomControl();
    zoomControl->endZoomSequence();
}

void TouchInputHandler::onUnblock() {
    this->primarySequence = nullptr;
    this->secondarySequence = nullptr;

    this->startZoomDistance = 0.0;
    this->lastZoomScrollCenter = {};
    this->lastAngle = 0;

    priLastAbs = {-1.0, -1.0};
    secLastAbs = {-1.0, -1.0};
    priLastRel = {-1.0, -1.0};
    secLastRel = {-1.0, -1.0};
}
