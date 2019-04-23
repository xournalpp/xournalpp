//
// Created by ulrich on 08.04.19.
//

#include "TouchDrawingInputHandler.h"
#include "InputContext.h"

#include <gui/widgets/XournalWidget.h>

TouchDrawingInputHandler::TouchDrawingInputHandler(InputContext* inputContext) : PenInputHandler(inputContext)
{

}bool TouchDrawingInputHandler::changeTool(GdkEvent* event)
{
	Settings* settings = this->inputContext->getSettings();
	ButtonConfig* cfgTouch = settings->getTouchButtonConfig();
	ToolHandler* toolHandler = this->inputContext->getToolHandler();
	GdkDevice* device = gdk_event_get_source_device(event);

	ButtonConfig* cfg = nullptr;
	if (cfgTouch->device == gdk_device_get_name(device))
	{
		cfg = cfgTouch;

		// If an action is defined we do it, even if it's a drawing action...
		if (cfg->getDisableDrawing() && cfg->getAction() == TOOL_NONE)
		{
			ToolType tool = toolHandler->getToolType();
			if (tool == TOOL_PEN || tool == TOOL_ERASER || tool == TOOL_HILIGHTER)
			{
				g_message("ignore touchscreen for drawing!\n");
				return true;
			}
		}
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

bool TouchDrawingInputHandler::handleImpl(GdkEvent* event)
{
	// Only handle events when there is no active gesture
	GtkXournal* xournal = inputContext->getXournal();
	if (xournal->view->getControl()->getWindow()->isGestureActive())
	{
		// Do not further relay events as they are of no interest
		return true;
	}

	GdkEventSequence* sequence = gdk_event_get_event_sequence(event);

	// Disallow multitouch
	if (this->currentSequence && this->currentSequence != sequence)
	{
		return false;
	}

	/*
	 * Trigger start action
	 */
	// Trigger start of action when pen/mouse is pressed
	if (event->type == GDK_TOUCH_BEGIN && this->currentSequence == nullptr)
	{
		this->currentSequence = sequence;
		this->deviceClassPressed = true;
		this->actionStart(event);
		return true;
	}

	/*
	 * Trigger motion actions
	 */
	// Trigger motion action when finger is pressed and moved
	if (this->deviceClassPressed && event->type == GDK_TOUCH_UPDATE)
	{
		this->actionMotion(event);
	}

	// Notify if finger enters/leaves widget
	if (event->type == GDK_ENTER_NOTIFY)
	{
		this->actionEnterWindow(event);
	}
	if (event->type == GDK_LEAVE_NOTIFY)
	{
		this->actionLeaveWindow(event);
	}

	// Trigger end of action if mouse button is released
	if (event->type == GDK_TOUCH_END || event->type == GDK_TOUCH_CANCEL)
	{
		this->currentSequence = nullptr;
		this->actionEnd(event);
		this->deviceClassPressed = false;
		return true;
	}

	// If we loose our Grab on the device end the current action
	if (event->type == GDK_GRAB_BROKEN && this->deviceClassPressed)
	{
		this->currentSequence = nullptr;
		this->actionEnd(event);
		this->deviceClassPressed = false;
		return true;
	}

	return false;
}
