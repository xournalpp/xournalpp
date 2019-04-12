//
// Created by ulrich on 08.04.19.
//

#include <gui/widgets/XournalWidget.h>
#include "StylusInputHandler.h"

StylusInputHandler::StylusInputHandler(InputContext* inputContext) : PenInputHandler(inputContext)
{

}

StylusInputHandler::~StylusInputHandler() = default;

bool StylusInputHandler::handleImpl(GdkEvent* event)
{
	// Only handle events when there is no active gesture
	GtkXournal* xournal = inputContext->getXournal();
	if (xournal->view->getControl()->getWindow()->isGestureActive())
	{
		//TODO what to do if motion is detected while input is active?

		// Do not further relay events as they are of no interest
		return true;
	}

	// Determine the pressed states of devices and associate them to the current event
	setPressedState(event);

	// Block devices if pen is within proximity of screen
	if (event->type == GDK_PROXIMITY_IN)
	{
		this->inputContext->blockDevice(InputContext::TOUCHSCREEN);
		this->inputContext->blockDevice(InputContext::MOUSE);
	}

	// Trigger start of action when pen/mouse is pressed
	if (event->type == GDK_BUTTON_PRESS)
	{
		guint button;
		gdk_event_get_button(event, &button);

		if (button == 1)
		{
			this->inputContext->blockDevice(InputContext::TOUCHSCREEN);
			this->inputContext->blockDevice(InputContext::MOUSE);
			this->actionStart(event);
			return true;
		}
	}

	// Trigger motion action when pen/mouse is pressed and moved
	if (this->deviceClassPressed && event->type == GDK_MOTION_NOTIFY) //mouse or pen moved
	{
		this->actionMotion(event);
	}

	// Scroll view if mouse or pen is pressed and leaves screen
	if (this->deviceClassPressed && event->type == GDK_ENTER_NOTIFY)
	{
		this->actionEnterWindow(event);
	}


	if (this->deviceClassPressed && event->type == GDK_LEAVE_NOTIFY)
	{
		this->actionLeaveWindow(event);
	}

	// Trigger end of action if pen tip leaves screen or mouse button is released
	if (event->type == GDK_BUTTON_RELEASE)
	{
		guint button;
		gdk_event_get_button(event, &button);

		if (button == 1)
		{
			this->actionEnd(event);
			this->inputContext->unblockDevice(InputContext::TOUCHSCREEN);
			this->inputContext->unblockDevice(InputContext::MOUSE);
			return true;
		}
	}

	// If we loose our Grab on the device end the current action
	if (event->type == GDK_GRAB_BROKEN && this->deviceClassPressed)
	{
		// TODO: We may need to update pressed state manually here
		this->actionEnd(event);
		this->inputContext->unblockDevice(InputContext::TOUCHSCREEN);
		this->inputContext->unblockDevice(InputContext::MOUSE);
		return true;
	}

	// Unblock devices if pen is out of proximity (required if pen never touches screen)
	if (event->type == GDK_PROXIMITY_OUT)
	{
		this->inputContext->unblockDevice(InputContext::TOUCHSCREEN);
		this->inputContext->unblockDevice(InputContext::MOUSE);
		return true;
	}

	return false;
}

bool StylusInputHandler::changeTool(GdkEvent* event)
{
	Settings* settings = this->inputContext->getSettings();
	ToolHandler* toolHandler = this->inputContext->getToolHandler();
	GdkDevice* device = gdk_event_get_source_device(event);

	ButtonConfig* cfg = nullptr;
	//Stylus
	if (gdk_device_get_source(device) == GDK_SOURCE_PEN)
	{
		if (this->modifier2)
		{
			cfg = settings->getStylusButton1Config();
		}
		else if (this->modifier3)
		{
			cfg = settings->getStylusButton2Config();
		}
	}
	else if (gdk_device_get_source(device) == GDK_SOURCE_ERASER)
	{
		cfg = settings->getEraserButtonConfig();
	}

	if (cfg && cfg->getAction() != TOOL_NONE)
	{
		toolHandler->copyCurrentConfig();
		cfg->acceptActions(toolHandler);
	}
	else
	{
		toolHandler->restoreLastConfig();
	}

	return false;
}
