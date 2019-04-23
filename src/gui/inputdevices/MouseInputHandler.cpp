//
// Created by ulrich on 08.04.19.
//

#include <gui/widgets/XournalWidget.h>
#include "MouseInputHandler.h"

MouseInputHandler::MouseInputHandler(InputContext* inputContext) : PenInputHandler(inputContext)
{

}

MouseInputHandler::~MouseInputHandler() = default;

bool MouseInputHandler::handleImpl(GdkEvent* event)
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

	/*
	 * Trigger start action
	 */
	// Trigger start of action when pen/mouse is pressed
	if (event->type == GDK_BUTTON_PRESS)
	{
		guint button;
		gdk_event_get_button(event, &button);

		if (button == 1)
		{
			this->actionStart(event);
			return true;
		}
	}

	/*
	 * Trigger motion actions
	 */
	// Trigger motion action when pen/mouse is pressed and moved
	if (this->deviceClassPressed && event->type == GDK_MOTION_NOTIFY) //mouse or pen moved
	{
		this->actionMotion(event);
	}

	// Notify if mouse enters/leaves widget
	if (event->type == GDK_ENTER_NOTIFY)
	{
		this->actionEnterWindow(event);
	}
	if (event->type == GDK_LEAVE_NOTIFY)
	{
		//this->inputContext->unblockDevice(InputContext::TOUCHSCREEN);
		//this->inputContext->getView()->getHandRecognition()->unblock();
		this->actionLeaveWindow(event);
	}

	// Trigger end of action if mouse button is released
	if (event->type == GDK_BUTTON_RELEASE)
	{
		guint button;
		gdk_event_get_button(event, &button);

		if (button == 1)
		{
			this->actionEnd(event);
			return true;
		}
	}

	// If we loose our Grab on the device end the current action
	if (event->type == GDK_GRAB_BROKEN && this->deviceClassPressed)
	{
		// TODO: We may need to update pressed state manually here
		this->actionEnd(event);
		return true;
	}

	return false;
}

bool MouseInputHandler::changeTool(GdkEvent* event)
{
	Settings* settings = this->inputContext->getSettings();
	ToolHandler* toolHandler = this->inputContext->getToolHandler();
	GtkXournal* xournal = this->inputContext->getXournal();

	ButtonConfig* cfg = nullptr;
	if (modifier2 /* Middle Button */ && !xournal->selection)
	{
		cfg = settings->getMiddleButtonConfig();
	}
	else if (modifier3 /* Right Button */ && !xournal->selection)
	{
		cfg = settings->getRightButtonConfig();
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

void MouseInputHandler::onBlock()
{

}
