//
// Created by ulrich on 08.04.19.
//

#include "TouchInputHandler.h"
#include "InputContext.h"

TouchInputHandler::TouchInputHandler(InputContext* inputContext) : AbstractInputHandler(inputContext)
{
	XOJ_INIT_TYPE(TouchInputHandler);
}

TouchInputHandler::~TouchInputHandler()
{
	XOJ_CHECK_TYPE(TouchInputHandler);

	XOJ_RELEASE_TYPE(TouchInputHandler);
}

bool TouchInputHandler::handleImpl(InputEvent* event)
{
	XOJ_CHECK_TYPE(TouchInputHandler);

	// Disallow multitouch
	if (this->currentSequence && this->currentSequence != event->sequence)
	{
		return false;
	}

	if (event->type == BUTTON_PRESS_EVENT && this->currentSequence == nullptr)
	{
		this->currentSequence = event->sequence;
		actionStart(event);
	}

	if (event->type == MOTION_EVENT)
	{
		actionMotion(event);
	}

	if (event->type == BUTTON_RELEASE_EVENT)
	{
		actionEnd(event);
		this->currentSequence = nullptr;
	}

	return false;
}

void TouchInputHandler::actionStart(InputEvent* event)
{
	XOJ_CHECK_TYPE(TouchInputHandler);

	this->lastPosX = event->absoluteX;
	this->lastPosY = event->absoluteY;

}

void TouchInputHandler::actionMotion(InputEvent* event)
{
	XOJ_CHECK_TYPE(TouchInputHandler);

	// Manually scroll when gesture is active
	if (this->inputContext->getView()->getZoomGestureHandler()->isGestureActive())
	{

		double offsetX = event->absoluteX - this->lastPosX;
		double offsetY = event->absoluteY - this->lastPosY;

		this->lastPosX = event->absoluteX;
		this->lastPosY = event->absoluteY;

		ZoomControl* zoomControl = this->inputContext->getView()->getControl()->getZoomControl();
		std::tuple<double, double> pos = zoomControl->getScrollPositionAfterZoom();

		double newX = std::get<0>(pos) - offsetX;
		double newY = std::get<1>(pos) - offsetY;

		zoomControl->setScrollPositionAfterZoom(newX, newY);
	}

	//  Manually scroll if non-touchscreen device was mapped to a touchscreen (GTK wont handle this)
	if (this->lastPosX >= 0.0 && this->lastPosY >= 0.0
			&& event->deviceClass == INPUT_DEVICE_TOUCHSCREEN && gdk_device_get_source(gdk_event_get_source_device(event->sourceEvent)) != GDK_SOURCE_TOUCHSCREEN)
	{

		double offsetX = event->absoluteX - this->lastPosX;
		double offsetY = event->absoluteY - this->lastPosY;

		this->lastPosX = event->absoluteX;
		this->lastPosY = event->absoluteY;

		GtkAdjustment* h = this->inputContext->getView()->getScrollHandling()->getHorizontal();
		gtk_adjustment_set_value(h, gtk_adjustment_get_value(h) - offsetX);
		GtkAdjustment* v = this->inputContext->getView()->getScrollHandling()->getVertical();
		gtk_adjustment_set_value(v, gtk_adjustment_get_value(v) - offsetY);
	}
}

void TouchInputHandler::actionEnd(InputEvent* event)
{
	XOJ_CHECK_TYPE(TouchInputHandler);

	this->lastPosX = -1.0;
	this->lastPosY = -1.0;
}


