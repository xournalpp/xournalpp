//
// Created by ulrich on 08.04.19.
//

#include "TouchInputHandler.h"
#include "InputContext.h"

TouchInputHandler::TouchInputHandler(InputContext* inputContext) : AbstractInputHandler(inputContext)
{
}

bool TouchInputHandler::handleImpl(InputEvent* event)
{
	// Don't handle more then 2 inputs
	if (this->primarySequence && this->primarySequence != event->sequence
		&& this->secondarySequence && this->secondarySequence != event->sequence)
	{
		return false;
	}

	if (event->type == BUTTON_PRESS_EVENT)
	{
		// Start scrolling when a sequence starts and we currently have none other
		if (this->primarySequence == nullptr && this->secondarySequence == nullptr)
		{
			this->primarySequence = event->sequence;

			// Set sequence data
			sequenceStart(event);
		}
		// Start zooming as soon as we have two sequences
		else if (this->primarySequence && this->primarySequence != event->sequence && this->secondarySequence == nullptr)
		{
			this->secondarySequence = event->sequence;

			// Set sequence data
			sequenceStart(event);

			zoomStart();
		}
	}

	if (event->type == MOTION_EVENT && this->primarySequence)
	{
		// Only zoom if there are two fingers involved
		if (this->primarySequence && this->secondarySequence)
		{
			zoomMotion(event);
		} else
		{
			scrollMotion(event);
		}
	}

	if (event->type == BUTTON_RELEASE_EVENT)
	{
		// Only stop zooing if both sequences were active (we were scrolling)
		if (this->primarySequence != nullptr && this->secondarySequence != nullptr)
		{
			zoomEnd();
		}

		if (event->sequence == this->primarySequence)
		{
			this->primarySequence = nullptr;
		} else {
			this->secondarySequence = nullptr;
		}
	}

	return false;
}

void TouchInputHandler::sequenceStart(InputEvent* event)
{
	if (event->sequence == this->primarySequence)
	{
		this->priLastAbsX = event->absoluteX;
		this->priLastAbsY = event->absoluteY;
		this->priLastRelX = event->relativeX;
		this->priLastRelY = event->relativeY;
	} else {
		this->secLastAbsX = event->absoluteX;
		this->secLastAbsY = event->absoluteY;
		this->secLastRelX = event->relativeX;
		this->secLastRelY = event->relativeY;
	}
}

void TouchInputHandler::scrollMotion(InputEvent* event)
{
	double offsetX;
	double offsetY;

	// Will only be called if there is a single sequence (zooming handles two sequences)
	if (event->sequence == this->primarySequence)
	{
		offsetX = event->absoluteX - this->priLastAbsX;
		offsetY = event->absoluteY - this->priLastAbsY;
		this->priLastAbsX = event->absoluteX;
		this->priLastAbsY = event->absoluteY;
	} else {
		offsetX = event->absoluteX - this->secLastAbsX;
		offsetY = event->absoluteY - this->secLastAbsY;
		this->secLastAbsX = event->absoluteX;
		this->secLastAbsY = event->absoluteY;
	}

	GtkAdjustment* h = this->inputContext->getView()->getScrollHandling()->getHorizontal();
	gtk_adjustment_set_value(h, gtk_adjustment_get_value(h) - offsetX);
	GtkAdjustment* v = this->inputContext->getView()->getScrollHandling()->getVertical();
	gtk_adjustment_set_value(v, gtk_adjustment_get_value(v) - offsetY);
}

void TouchInputHandler::zoomStart()
{
	if (!inputContext->getSettings()->isZoomGesturesEnabled())
	{
		return;
	}

	// Take horizontal and vertical padding of view into account when calculating the center of the gesture
	int vPadding = inputContext->getSettings()->getAddVerticalSpace() ? inputContext->getSettings()->getAddVerticalSpaceAmount() : 0;
	int hPadding = inputContext->getSettings()->getAddHorizontalSpace() ? inputContext->getSettings()->getAddHorizontalSpaceAmount() : 0;

	double centerX = (this->priLastRelX + this->secLastRelX) / 2.0 - hPadding;
	double centerY = (this->priLastRelY + this->secLastRelY) / 2.0 - vPadding;

	this->startZoomDistance = std::sqrt(std::pow(this->priLastAbsX - this->secLastAbsX, 2.0) + std::pow(this->priLastAbsY - this->secLastAbsY, 2.0));
	lastZoomScrollCenterX = (this->priLastAbsX + this->secLastAbsX) / 2.0;
	lastZoomScrollCenterY = (this->priLastAbsY + this->secLastAbsY) / 2.0;

	ZoomControl* zoomControl = this->inputContext->getView()->getControl()->getZoomControl();

	// Disable zoom fit as we are zooming currently
	// TODO this should happen internally!!!
	if(zoomControl->isZoomFitMode())
	{
		zoomControl->setZoomFitMode(false);
	}

	Rectangle zoomSequenceRectangle = zoomControl->getVisibleRect();

	zoomControl->startZoomSequence(centerX - zoomSequenceRectangle.x, centerY - zoomSequenceRectangle.y);
}

void TouchInputHandler::zoomMotion(InputEvent* event)
{

	if (event->sequence == this->primarySequence)
	{
		this->priLastAbsX = event->absoluteX;
		this->priLastAbsY = event->absoluteY;
	} else {
		this->secLastAbsX = event->absoluteX;
		this->secLastAbsY = event->absoluteY;
	}

	double sqDistance = std::sqrt(std::pow(this->priLastAbsX - this->secLastAbsX, 2.0) + std::pow(this->priLastAbsY - this->secLastAbsY, 2.0));
	double zoom = sqDistance / this->startZoomDistance;

	ZoomControl* zoomControl = this->inputContext->getView()->getControl()->getZoomControl();
	zoomControl->zoomSequenceChange(zoom, true);

	double centerX = (this->priLastAbsX + this->secLastAbsX) / 2.0;
	double centerY = (this->priLastAbsY + this->secLastAbsY) / 2.0;

	std::tuple<double, double> lastScrollPosition = zoomControl->getScrollPositionAfterZoom();
	double offsetX = std::get<0>(lastScrollPosition) - (centerX - lastZoomScrollCenterX);
	double offsetY = std::get<1>(lastScrollPosition) - (centerY - lastZoomScrollCenterY);

	zoomControl->setScrollPositionAfterZoom(offsetX, offsetY);
	lastZoomScrollCenterX = centerX;
	lastZoomScrollCenterY = centerY;
}

void TouchInputHandler::zoomEnd()
{
	ZoomControl* zoomControl = this->inputContext->getView()->getControl()->getZoomControl();
	zoomControl->endZoomSequence();
}
