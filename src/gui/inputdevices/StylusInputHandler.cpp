//
// Created by ulrich on 08.04.19.
//

#include "StylusInputHandler.h"
#include "InputContext.h"

#include <gui/widgets/XournalWidget.h>
#include "gui/XournalppCursor.h"
#include <cmath>

StylusInputHandler::StylusInputHandler(InputContext* inputContext) : PenInputHandler(inputContext)
{
	XOJ_INIT_TYPE(StylusInputHandler);
}

StylusInputHandler::~StylusInputHandler()
{
	XOJ_CHECK_TYPE(StylusInputHandler);

	XOJ_RELEASE_TYPE(StylusInputHandler);
}

bool StylusInputHandler::handleImpl(GdkEvent* event)
{
	XOJ_CHECK_TYPE(StylusInputHandler);

	// Only handle events when there is no active gesture
	GtkXournal* xournal = inputContext->getXournal();
	if (xournal->view->getControl()->getWindow()->isGestureActive())
	{
		// Do not further relay events as they are of no interest
		return true;
	}

	// Determine the pressed states of devices and associate them to the current event
	setPressedState(event);

	// Trigger start of action when pen/mouse is pressed
	if (event->type == GDK_BUTTON_PRESS)
	{
		guint button;
		gdk_event_get_button(event, &button);

		if (button == 1 || this->inputContext->getSettings()->getInputSystemTPCButtonEnabled())
		{
			this->actionStart(event);
			return true;
		} else if (this->inputRunning)
		{
			// TPCButton is disabled and modifier button was pressed
			this->actionEnd(event);
			this->actionStart(event);
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
	if (event->type == GDK_ENTER_NOTIFY && !this->inputRunning)
	{
		this->actionEnterWindow(event);
	}
	if (event->type == GDK_LEAVE_NOTIFY)
	{
		this->inputContext->getView()->getHandRecognition()->unblock();
		this->actionLeaveWindow(event);
	}

	// Trigger end of action if pen tip leaves screen or mouse button is released
	if (event->type == GDK_BUTTON_RELEASE)
	{
		guint button;
		gdk_event_get_button(event, &button);

		if (button == 1 || this->inputContext->getSettings()->getInputSystemTPCButtonEnabled())
		{
			this->actionEnd(event);
			return true;
		} else if (this->inputRunning)
		{
			// TPCButton is disabled and modifier button was released
			this->actionEnd(event);
			this->actionStart(event);
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

void StylusInputHandler::setPressedState(GdkEvent* event)
{
	XOJ_CHECK_TYPE(StylusInputHandler);

	XojPageView* currentPage = getPageAtCurrentPosition(event);

	this->inputContext->getXournal()->view->getCursor()->setInsidePage(currentPage != nullptr);

	if (event->type == GDK_BUTTON_PRESS) //mouse button pressed or pen touching surface
	{
		guint button;
		gdk_event_get_button(event, &button);

		switch (button)
		{
			case 1:
				this->deviceClassPressed = true;
				break;
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

		switch (button)
		{
			case 1:
				this->deviceClassPressed = false;
				break;
			case 2:
				this->modifier2 = false;
				break;
			case 3:
				this->modifier3 = false;
			default:
				break;
		}
	}

	if (this->inputContext->getSettings()->getInputSystemTPCButtonEnabled())
	{
		this->deviceClassPressed = this->deviceClassPressed || this->modifier2 || this->modifier3;
	}
}

bool StylusInputHandler::changeTool(GdkEvent* event)
{
	XOJ_CHECK_TYPE(StylusInputHandler);

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

