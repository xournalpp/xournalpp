//
// Created by ulrich on 08.04.19.
//

#include "TouchInputHandler.h"
#include "InputContext.h"

TouchInputHandler::TouchInputHandler(InputContext* inputContext) : AbstractInputHandler(inputContext) = default;

TouchInputHandler::~TouchInputHandler() = default;

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
		if (this->primarySequence == nullptr)
		{
			this->primarySequence = event->sequence;
			actionStart(event);
		}

		if (this->primarySequence && this->primarySequence != event->sequence && this->secondarySequence == nullptr)
		{
			this->secondarySequence = event->sequence;
			zoomStart(event);
		}
	}

	if (event->type == MOTION_EVENT && this->primarySequence)
	{
		if (this->secondarySequence)
		{
			zoomMotion(event);
		} else {
			actionMotion(event);
		}
	}

	if (event->type == BUTTON_RELEASE_EVENT)
	{
		actionEnd(event);
		this->primarySequence = nullptr;
		this->secondarySequence = nullptr;
	}

	return false;
}

void TouchInputHandler::actionStart(InputEvent* event)
{
	this->priLastPosX = event->absoluteX;
	this->priLastPosY = event->absoluteY;

}

void TouchInputHandler::actionMotion(InputEvent* event)
{
	// Manually scroll when gesture is active
	if (this->inputContext->getView()->getZoomGestureHandler()->isGestureActive())
	{

		double offsetX = event->absoluteX - this->priLastPosX;
		double offsetY = event->absoluteY - this->priLastPosY;

		this->priLastPosX = event->absoluteX;
		this->priLastPosY = event->absoluteY;

		ZoomControl* zoomControl = this->inputContext->getView()->getControl()->getZoomControl();
		std::tuple<double, double> pos = zoomControl->getScrollPositionAfterZoom();

		double newX = std::get<0>(pos) - offsetX;
		double newY = std::get<1>(pos) - offsetY;

		zoomControl->setScrollPositionAfterZoom(newX, newY);
	}

	//  Manually scroll if non-touchscreen device was mapped to a touchscreen (GTK wont handle this)
	if (this->priLastPosX >= 0.0 && this->priLastPosY >= 0.0
			&& event->deviceClass == INPUT_DEVICE_TOUCHSCREEN && gdk_device_get_source(gdk_event_get_source_device(event->sourceEvent)) != GDK_SOURCE_TOUCHSCREEN)
	{

		double offsetX = event->absoluteX - this->priLastPosX;
		double offsetY = event->absoluteY - this->priLastPosY;

		this->priLastPosX = event->absoluteX;
		this->priLastPosY = event->absoluteY;

		GtkAdjustment* h = this->inputContext->getView()->getScrollHandling()->getHorizontal();
		gtk_adjustment_set_value(h, gtk_adjustment_get_value(h) - offsetX);
		GtkAdjustment* v = this->inputContext->getView()->getScrollHandling()->getVertical();
		gtk_adjustment_set_value(v, gtk_adjustment_get_value(v) - offsetY);
	}
}

void TouchInputHandler::actionEnd(InputEvent* event)
{
	this->priLastPosX = -1.0;
	this->priLastPosY = -1.0;
	this->secLastPosX = -1.0;
	this->secLastPosX = -1.0;
	this->lastDiff = 0.0;

	if (this->secondarySequence)
	{
		ZoomControl* zoomControl = this->inputContext->getView()->getControl()->getZoomControl();
		zoomControl->endZoomSequence();
	}
}

void TouchInputHandler::zoomStart(InputEvent* event)
{
	this->secLastPosX = event->absoluteX;
	this->secLastPosY = event->absoluteY;

	double centerX = (this->priLastPosX + this->secLastPosX) / 2;
	double centerY = (this->priLastPosY + this->secLastPosY) / 2;

	ZoomControl* zoomControl = this->inputContext->getView()->getControl()->getZoomControl();

	if(zoomControl->isZoomFitMode())
	{
		zoomControl->setZoomFitMode(false);
	}

	zoomControl->startZoomSequence(centerX, centerY);
}

void TouchInputHandler::zoomMotion(InputEvent* event)
{
	double sqOldDistance = std::pow(this->priLastPosX - this->secLastPosX, 2) + std::pow(this->priLastPosY- this->secLastPosY, 2);

	if (event->sequence == this->primarySequence)
	{
		this->priLastPosX = event->absoluteX;
		this->priLastPosY = event->absoluteY;
	} else {
		this->secLastPosX = event->absoluteX;
		this->secLastPosY = event->absoluteY;
	}

	double sqNewDistance = std::pow(this->priLastPosX - this->secLastPosX, 2) + std::pow(this->priLastPosY- this->secLastPosY, 2);

	double diff = sqNewDistance - sqOldDistance;

	if (diff == 0.0)
	{
		return;
	}

	if (this->lastDiff == 0.0)
	{
		this->lastDiff = diff;
		return;
	}

	ZoomControl* zoomControl = this->inputContext->getView()->getControl()->getZoomControl();

	// TODO: Better algorithm for the zoom value
	zoomControl->zoomSequenceChange(zoomControl->getZoom() + diff/80000, false);
}
