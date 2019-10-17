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
}

StylusInputHandler::~StylusInputHandler()
{
}

bool StylusInputHandler::handleImpl(InputEvent* event)
{
	// Only handle events when there is no active gesture
	GtkXournal* xournal = inputContext->getXournal();

	// Determine the pressed states of devices and associate them to the current event
	setPressedState(event);

	// Trigger start of action when pen/mouse is pressed
	if (event->type == BUTTON_PRESS_EVENT)
	{

		if (event->button == 1 || this->inputContext->getSettings()->getInputSystemTPCButtonEnabled())
		{
			this->actionStart(event);
			return true;
		} else if (this->inputRunning)
		{
			// TPCButton is disabled and modifier button was pressed
			this->actionEnd(event);
			this->actionStart(event);
		} else
		{
			// No input running but modifier key was pressed
			// Change the tool depending on the key
			changeTool(event);
		}
	}

	// Trigger discrete action on double tap
	if (event->type == BUTTON_2_PRESS_EVENT || event->type == BUTTON_3_PRESS_EVENT)
	{
		this->actionPerform(event);
		return true;
	}

	// Trigger motion action when pen/mouse is pressed and moved
	if (event->type == MOTION_EVENT) //mouse or pen moved
	{
		this->actionMotion(event);
		XournalppCursor* cursor = xournal->view->getCursor();
		cursor->setInvisible(false);
		cursor->updateCursor();
	}


	// Check if enter/leave events occur in possible locations. This is a bug of the hardware (there are such devices!)
	if ((event->type == ENTER_EVENT || event->type == LEAVE_EVENT) && this->deviceClassPressed && this->lastEvent)
	{
		if (std::abs(event->relativeX - lastEvent->relativeX) > 100 || std::abs(event->relativeY - lastEvent->relativeY) > 100)
		{
			g_message("Discard impossible event - this is a sign of bugged hardware or drivers");
			return true;
		}
	}

	// Notify if pen enters/leaves widget
	if (event->type == ENTER_EVENT && !this->inputRunning)
	{
		this->actionEnterWindow(event);
	}
	if (event->type == LEAVE_EVENT)
	{
		this->inputContext->getView()->getHandRecognition()->unblock();
		this->actionLeaveWindow(event);
	}

	// Trigger end of action if pen tip leaves screen or mouse button is released
	if (event->type == BUTTON_RELEASE_EVENT)
	{
		if (event->button == 1 || this->inputContext->getSettings()->getInputSystemTPCButtonEnabled())
		{
			this->actionEnd(event);
			return true;
		} else if (this->inputRunning)
		{
			// TPCButton is disabled and modifier button was released
			this->actionEnd(event);
			this->actionStart(event);
		} else
		{
			// No input running but modifier key was pressed
			// Change the tool depending on the key
			changeTool(event);
		}
	}

	// If we loose our Grab on the device end the current action
	if (event->type == GRAB_BROKEN_EVENT && this->deviceClassPressed)
	{
		// TODO: We may need to update pressed state manually here
		this->actionEnd(event);
		return true;
	}

	return false;
}

void StylusInputHandler::setPressedState(InputEvent* event)
{
	XojPageView* currentPage = getPageAtCurrentPosition(event);

	this->inputContext->getXournal()->view->getCursor()->setInsidePage(currentPage != nullptr);

	if (event->type == BUTTON_PRESS_EVENT) //mouse button pressed or pen touching surface
	{
		switch (event->button)
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
	if (event->type == BUTTON_RELEASE_EVENT) //mouse button released or pen not touching surface anymore
	{
		switch (event->button)
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

bool StylusInputHandler::changeTool(InputEvent* event)
{
	Settings* settings = this->inputContext->getSettings();
	ToolHandler* toolHandler = this->inputContext->getToolHandler();

	ButtonConfig* cfg = nullptr;
	//Stylus
	if (event->deviceClass == INPUT_DEVICE_PEN)
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
	else if (event->deviceClass == INPUT_DEVICE_ERASER)
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

