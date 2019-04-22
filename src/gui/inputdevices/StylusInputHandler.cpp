//
// Created by ulrich on 08.04.19.
//

#include <gui/widgets/XournalWidget.h>
#include "gui/Cursor.h"
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
		// Do not further relay events as they are of no interest
		return true;
	}

	// Determine the pressed states of devices and associate them to the current event
	setPressedState(event);

	// Block devices if pen is within proximity of screen
	if (event->type == GDK_PROXIMITY_IN)
	{
		this->blockDevices();
	}

	// Trigger start of action when pen/mouse is pressed
	if (event->type == GDK_BUTTON_PRESS)
	{
		guint button;
		gdk_event_get_button(event, &button);

		if (button == 1)
		{
			this->blockDevices();
			this->actionStart(event);
			return true;
		}
	}

	// Trigger motion action when pen/mouse is pressed and moved
	if (this->deviceClassPressed && event->type == GDK_MOTION_NOTIFY) //mouse or pen moved
	{
		this->actionMotion(event);
	}


	// Check if enter/leave events occur in possible locations. This is a bug of the hardware (there are such devices!)
	if ((event->type == GDK_ENTER_NOTIFY || event->type == GDK_LEAVE_NOTIFY) && this->deviceClassPressed && this->lastEvent)
	{
		gdouble lastX, lastY, currentX, currentY;
		gdk_event_get_coords(this->lastEvent, &lastX, &lastY);
		gdk_event_get_coords(event, &currentX, &currentY);

		if (std::abs(currentX - lastX) > 100 || std::abs(currentY - lastY) > 100)
		{
			g_message("Discard impossible event - this is a sign of bugged hardware or drivers");
			return true;
		}
	}

	// Notify if pen enters/leaves widget
	//TODO how to handle sequences with enter but no leave event in terms of blocked devices?
	if (event->type == GDK_ENTER_NOTIFY && !this->inputRunning)
	{
		this->blockDevices();
		this->actionEnterWindow(event);
	}
	if (event->type == GDK_LEAVE_NOTIFY)
	{
		this->unblockDevices();
		this->inputContext->getView()->getHandRecognition()->unblock();
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
			this->unblockDevices();
			return true;
		}
	}

	// If we loose our Grab on the device end the current action
	if (event->type == GDK_GRAB_BROKEN && this->deviceClassPressed)
	{
		// TODO: We may need to update pressed state manually here
		this->actionEnd(event);
		this->unblockDevices();
		this->inputContext->getView()->getHandRecognition()->unblock();
		return true;
	}

	// Unblock devices if pen is out of proximity (required if pen never touches screen)
	if (event->type == GDK_PROXIMITY_OUT)
	{
		this->unblockDevices();
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

void StylusInputHandler::blockDevices()
{
	this->inputContext->blockDevice(InputContext::MOUSE);
	this->inputContext->blockDevice(InputContext::TOUCHSCREEN);
}

void StylusInputHandler::unblockDevices()
{
	this->inputContext->unblockDevice(InputContext::MOUSE);
	this->inputContext->unblockDevice(InputContext::TOUCHSCREEN);
}
