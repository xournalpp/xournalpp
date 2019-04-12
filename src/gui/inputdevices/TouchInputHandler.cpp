//
// Created by ulrich on 08.04.19.
//

#include "TouchInputHandler.h"

TouchInputHandler::TouchInputHandler(InputContext* inputContext) : AbstractInputHandler(inputContext)
{

}

bool TouchInputHandler::handleImpl(GdkEvent* event)
{
	return false;
}


