//
// Created by ulrich on 08.04.19.
//

#include <gui/widgets/XournalWidget.h>
#include "MouseInputHandler.h"
#include "InputContext.h"
#include <gui/XournalppCursor.h>

MouseInputHandler::MouseInputHandler(InputContext* inputContext) : PenInputHandler(inputContext)
{
	XOJ_INIT_TYPE(MouseInputHandler);
}

MouseInputHandler::~MouseInputHandler()
{
	XOJ_CHECK_TYPE(MouseInputHandler);

	XOJ_RELEASE_TYPE(MouseInputHandler);
}

bool MouseInputHandler::handleImpl(GdkEvent* event)
{
	XOJ_CHECK_TYPE(MouseInputHandler);

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

		this->actionStart(event);
		return true;
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

		this->actionEnd(event);
		return true;
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

void MouseInputHandler::setPressedState(GdkEvent* event)
{
	XOJ_CHECK_TYPE(MouseInputHandler);

	XojPageView* currentPage = getPageAtCurrentPosition(event);

	this->inputContext->getXournal()->view->getCursor()->setInsidePage(currentPage != nullptr);

	if (event->type == GDK_BUTTON_PRESS) //mouse button pressed or pen touching surface
	{
		guint button;
		gdk_event_get_button(event, &button);

		this->deviceClassPressed = true;

		switch (button)
		{
			case 2:
				this->modifier2 = true;
				break;
			case 3:
				this->modifier3 = true;
			default:
				break;
		}
	}
	if (event->type == GDK_BUTTON_RELEASE) //mouse button released or pen not touching surface anymore
	{
		guint button;
		gdk_event_get_button(event, &button);

		this->deviceClassPressed = false;

		switch (button)
		{
			case 2:
				this->modifier2 = false;
				break;
			case 3:
				this->modifier3 = false;
			default:
				break;
		}
	}
}

bool MouseInputHandler::changeTool(GdkEvent* event)
{
	XOJ_CHECK_TYPE(MouseInputHandler);

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
	XOJ_CHECK_TYPE(MouseInputHandler);
}
