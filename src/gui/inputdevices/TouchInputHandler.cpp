//
// Created by ulrich on 08.04.19.
//

#include "TouchInputHandler.h"

#include "InputContext.h"

TouchInputHandler::TouchInputHandler(InputContext* inputContext): AbstractInputHandler(inputContext) {}

auto TouchInputHandler::handleImpl(InputEvent const& event) -> bool {
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
            sequenceStart(event);
        }
        // Start zooming as soon as we have two sequences
        else if (this->primarySequence && this->primarySequence != event.sequence &&
                 this->secondarySequence == nullptr) {
            this->secondarySequence = event.sequence;

            // Set sequence data
            sequenceStart(event);

            zoomStart();
        }
    }

    if (event.type == MOTION_EVENT && this->primarySequence) {
        // Only zoom if there are two fingers involved
        if (this->primarySequence && this->secondarySequence) {
            zoomMotion(event);
        } else {
            scrollMotion(event);
        }
    }

    if (event.type == BUTTON_RELEASE_EVENT) {
        // Only stop zooing if both sequences were active (we were scrolling)
        if (this->primarySequence != nullptr && this->secondarySequence != nullptr) {
            zoomEnd();
        }

        if (event.sequence == this->primarySequence) {
            this->primarySequence = nullptr;
        } else {
            this->secondarySequence = nullptr;
        }
    }

    return false;
}

void TouchInputHandler::sequenceStart(InputEvent const& event) {
    if (event.sequence == this->primarySequence) {
        this->priLastAbs = {event.absoluteX, event.absoluteY};
        this->priLastRel = {event.relativeX, event.relativeY};
    } else {
        this->secLastAbs = {event.absoluteX, event.absoluteY};
        this->secLastRel = {event.relativeX, event.relativeY};
    }
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

    GtkAdjustment* h = this->inputContext->getView()->getScrollHandling()->getHorizontal();
    gtk_adjustment_set_value(h, gtk_adjustment_get_value(h) - offset.x);
    GtkAdjustment* v = this->inputContext->getView()->getScrollHandling()->getVertical();
    gtk_adjustment_set_value(v, gtk_adjustment_get_value(v) - offset.y);
}

void TouchInputHandler::zoomStart() {
    if (!inputContext->getSettings()->isZoomGesturesEnabled()) {
        return;
    }

    // Take horizontal and vertical padding of view into account when calculating the center of the gesture
    int vPadding = inputContext->getSettings()->getAddVerticalSpace() ?
                           inputContext->getSettings()->getAddVerticalSpaceAmount() :
                           0;
    int hPadding = inputContext->getSettings()->getAddHorizontalSpace() ?
                           inputContext->getSettings()->getAddHorizontalSpaceAmount() :
                           0;

    auto center = (this->priLastRel + this->secLastRel) / 2.0 - utl::Point<double>{double(hPadding), double(vPadding)};

    this->startZoomDistance = this->priLastAbs.distance(this->secLastAbs);
    lastZoomScrollCenter = (this->priLastAbs + this->secLastAbs) / 2.0;

    ZoomControl* zoomControl = this->inputContext->getView()->getControl()->getZoomControl();

    // Disable zoom fit as we are zooming currently
    // TODO(fabian): this should happen internally!!!
    if (zoomControl->isZoomFitMode()) {
        zoomControl->setZoomFitMode(false);
    }

    Rectangle zoomSequenceRectangle = zoomControl->getVisibleRect();

    zoomControl->startZoomSequence(center);
}

void TouchInputHandler::zoomMotion(InputEvent const& event) {

    if (event.sequence == this->primarySequence) {
        this->priLastAbs = {event.absoluteX, event.absoluteY};
    } else {
        this->secLastAbs = {event.absoluteX, event.absoluteY};
    }

    double sqDistance = this->priLastAbs.distance(this->secLastAbs);
    double zoom = sqDistance / this->startZoomDistance;

    ZoomControl* zoomControl = this->inputContext->getView()->getControl()->getZoomControl();
    zoomControl->zoomSequenceChange(zoom, true);

    auto center = (this->priLastAbs + this->secLastAbs) / 2;
    auto lastScrollPosition = zoomControl->getScrollPositionAfterZoom();
    auto offset = lastScrollPosition - (center - lastZoomScrollCenter);

    zoomControl->setScrollPositionAfterZoom(offset);
    lastZoomScrollCenter = center;
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

    priLastAbs = {-1.0, -1.0};
    secLastAbs = {-1.0, -1.0};
    priLastRel = {-1.0, -1.0};
    secLastRel = {-1.0, -1.0};
}
