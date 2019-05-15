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

bool TouchInputHandler::handleImpl(GdkEvent* event)
{
	XOJ_CHECK_TYPE(TouchInputHandler);

	GdkEventSequence* sequence = gdk_event_get_event_sequence(event);

	// Disallow multitouch
	if (this->currentSequence && this->currentSequence != sequence)
	{
		return false;
	}

	if (event->type == GDK_TOUCH_BEGIN && this->currentSequence == nullptr)
	{
		this->currentSequence = sequence;
		actionStart(event);
	}

	if (event->type == GDK_TOUCH_UPDATE)
	{
		actionMotion(event);
	}

	if (event->type == GDK_TOUCH_END || event->type == GDK_TOUCH_CANCEL)
	{
		actionEnd(event);
		this->currentSequence = nullptr;
	}

	return false;
}

void TouchInputHandler::actionStart(GdkEvent* event)
{
	XOJ_CHECK_TYPE(TouchInputHandler);

	gdk_event_get_root_coords(event, &this->lastPosX, &this->lastPosY);

}

void TouchInputHandler::actionMotion(GdkEvent* event)
{
	XOJ_CHECK_TYPE(TouchInputHandler);

	// Manually scroll when gesture is active
	if (this->inputContext->getView()->getZoomGestureHandler()->isGestureActive())
	{
		gdouble currentPosX, currentPosY;
		gdk_event_get_root_coords(event, &currentPosX, &currentPosY);

		double offsetX = currentPosX - this->lastPosX;
		double offsetY = currentPosY - this->lastPosY;

		this->lastPosX = currentPosX;
		this->lastPosY = currentPosY;

		ZoomControl* zoomControl = this->inputContext->getView()->getControl()->getZoomControl();
		std::tuple<double, double> pos = zoomControl->getScrollPositionAfterZoom();

		double newX = std::get<0>(pos) - offsetX;
		double newY = std::get<1>(pos) - offsetY;

		zoomControl->setScrollPositionAfterZoom(newX, newY);
	}
}

void TouchInputHandler::actionEnd(GdkEvent* event)
{
	XOJ_CHECK_TYPE(TouchInputHandler);
}


