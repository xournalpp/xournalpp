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

	return false;
}


