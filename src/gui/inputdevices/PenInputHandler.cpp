//
// Created by ulrich on 06.04.19.
//

#include "PenInputHandler.h"
#include "AbstractInputHandler.h"
#include "InputContext.h"

#include <gui/widgets/XournalWidget.h>
#include <control/ToolHandler.h>
#include <gui/XournalppCursor.h>
#include <gui/inputdevices/old/PositionInputData.h>
#include "gui/XournalView.h"
#include <control/settings/ButtonConfig.h>

#include <cmath>

PenInputHandler::PenInputHandler(InputContext* inputContext) : AbstractInputHandler(inputContext)
{

}

PenInputHandler::~PenInputHandler() = default;

void PenInputHandler::updateLastEvent(GdkEvent* event)
{
	if (!event)
	{
		return;
	}

	GdkEvent* oldEvent = this->lastEvent;
	this->lastEvent = gdk_event_copy(event);

	//Update the refcount of the events for the GTK garbage collector
	if (oldEvent != nullptr)
	{
		gdk_event_free(oldEvent);
	}

	if (getPageAtCurrentPosition(event))
	{
		GdkEvent* oldHitEvent = this->lastHitEvent;
		this->lastHitEvent = gdk_event_copy(event);

		//Update the refcount of the events for the GTK garbage collector
		if (oldHitEvent != nullptr)
		{
			gdk_event_free(oldHitEvent);
		}
	}
}

void PenInputHandler::setPressedState(GdkEvent* event)
{
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
}

void PenInputHandler::handleScrollEvent(GdkEvent* event)
{
	// use root coordinates as reference point because
	// scrolling changes window relative coordinates
	// see github Gnome/evince@1adce5486b10e763bed869

	gdouble rootX, rootY;
	gdk_event_get_root_coords(event, &rootX, &rootY);

	// GTK handles event compression/filtering differently between versions - this may be needed on certain hardware/GTK combinations.
	if (std::abs((double)(this->scrollStartX - rootX)) < 0.1 && std::abs((double)(this->scrollStartY - rootY)) < 0.1 )
	{
		return;
	}

	if (this->scrollOffsetX == 0 && this->scrollOffsetY == 0)
	{
		this->scrollOffsetX = this->scrollStartX - rootX;
		this->scrollOffsetY = this->scrollStartY - rootY;

		Util::execInUiThread([&]() {
			this->inputContext->getXournal()->layout->scrollRelative(this->scrollOffsetX, this->scrollOffsetY);

			// Scrolling done, so reset our counters
			this->scrollOffsetX = 0;
			this->scrollOffsetY = 0;
		});

		// Update the reference for the scroll-offset
		this->scrollStartX = rootX;
		this->scrollStartY = rootY;
	}
}

bool PenInputHandler::actionStart(GdkEvent* event)
{
	this->inputContext->focusWidget();

	XojPageView* currentPage = this->getPageAtCurrentPosition(event);
	// set reference data for handling of entering/leaving page
	this->updateLastEvent(event);

	// Flag running input
	this->inputRunning = true;
	this->penInWidget = true;

	// Change the tool depending on the key
	changeTool(event);

	GtkXournal* xournal = this->inputContext->getXournal();

	XournalppCursor* cursor = xournal->view->getCursor();
	cursor->setMouseDown(true);


	ToolHandler* toolHandler = this->inputContext->getToolHandler();

	// Save the starting offset when hand-tool is selected to get a reference for the scroll-offset
	if (toolHandler->getToolType() == TOOL_HAND)
	{
		gdk_event_get_root_coords(event, &this->scrollStartX, &this->scrollStartY);
	}

	// hand tool don't change the selection, so you can scroll e.g. with your touchscreen without remove the selection
	if (toolHandler->getToolType() != TOOL_HAND && xournal->selection)
	{
		EditSelection* selection = xournal->selection;

		XojPageView* view = selection->getView();
		PositionInputData selectionPos = this->getInputDataRelativeToCurrentPage(view, event);

		// Check if event modifies selection instead of page
		CursorSelectionType selType = selection->getSelectionTypeForPos(selectionPos.x, selectionPos.y, xournal->view->getZoom());
		if (selType)
		{

			if (selType == CURSOR_SELECTION_MOVE && modifier3)
			{
				selection->copySelection();
			}

			xournal->selection->mouseDown(selType, selectionPos.x, selectionPos.y);
			// Only modify selection and do not forward event to page
			return true;
		}
		else
		{
			xournal->view->clearSelection();
			if (changeTool(event))
			{
				// Do not handle event in any further way to make click only deselect selection
				return true;
			}
		}
	}

	// Forward event to page
	if (currentPage)
	{
		PositionInputData pos = this->getInputDataRelativeToCurrentPage(currentPage, event);
		return currentPage->onButtonPressEvent(pos);
	}

	// not handled
	return false;
}

bool PenInputHandler::actionMotion(GdkEvent* event)
{
	GtkXournal* xournal = this->inputContext->getXournal();
	ToolHandler* h = this->inputContext->getToolHandler();

	this->changeTool(event);

	if (h->getToolType() == TOOL_HAND)
	{
		if (this->deviceClassPressed)
		{
			this->handleScrollEvent(event);
			return true;
		}
		return false;
	}
	else if (xournal->selection)
	{
		EditSelection* selection = xournal->selection;
		XojPageView* view = selection->getView();

		PositionInputData pos = getInputDataRelativeToCurrentPage(view, event);

		if (xournal->selection->isMoving())
		{
			selection->mouseMove(pos.x, pos.y);
		}
		else
		{
			CursorSelectionType selType = selection->getSelectionTypeForPos(pos.x, pos.y, xournal->view->getZoom());
			xournal->view->getCursor()->setMouseSelectionType(selType);
		}
		return true;
	}

	// Check if page was left / entered
	XojPageView* lastEventPage = getPageAtCurrentPosition(this->lastEvent);
	XojPageView* lastHitEventPage = getPageAtCurrentPosition(this->lastHitEvent);
	XojPageView* currentPage = getPageAtCurrentPosition(event);

	/*
	 * Get all events where the input sequence moved from one page to another without stopping the input.
	 * Only trigger once the new page was entered to ensure that an input device can leave the page temporarily.
	 * For these events we need to fake an end point in the old page and a start point in the new page.
	 */
	if (currentPage && !lastEventPage && lastHitEventPage)
	{
#ifdef DEBUG_INPUT
		g_message("PenInputHandler: Start new input on switching page...");
#endif
		this->actionEnd(this->lastHitEvent);
		this->updateLastEvent(event);

		bool result =  this->actionStart(event);
		this->updateLastEvent(event);
		return result;
	}
	/*
	 * Get all events where the input sequence started outside of a page and moved into one.
	 * For these events we need to fake a start point in the current page.
	 */
	if (currentPage && !lastEventPage && !lastHitEventPage)
	{
#ifdef DEBUG_INPUT
		g_message("PenInputHandler: Start new input on entering page...");
#endif
		bool result =  this->actionStart(event);
		this->updateLastEvent(event);
		return result;
	}

	// Update the last position of the input device
	this->updateLastEvent(event);

	// Update the cursor
	xournal->view->getCursor()->setInsidePage(currentPage != nullptr);
	if (currentPage && this->penInWidget)
	{
		// Relay the event to the page
		PositionInputData pos = getInputDataRelativeToCurrentPage(currentPage, event);
		return currentPage->onMotionNotifyEvent(pos);
	} else
	{
		return false;
	}
}

bool PenInputHandler::actionEnd(GdkEvent* event)
{
	GtkXournal* xournal = inputContext->getXournal();
	XournalppCursor* cursor = xournal->view->getCursor();
	ToolHandler* toolHandler = inputContext->getToolHandler();

	cursor->setMouseDown(false);

	EditSelection* sel = xournal->view->getSelection();
	if (sel)
	{
		sel->mouseUp();
	}

	//Relay the event to the page
	XojPageView* currentPage = getPageAtCurrentPosition(event);

	/*
	 * Use the last active page if you can't find a page under the cursor position.
	 * This is a workaround for input leaving the page while being active and then stopping outside.
	 */
	if (!currentPage)
	{
		if (!this->lastHitEvent)
		{
			return false;
		}
		currentPage = getPageAtCurrentPosition(this->lastHitEvent);
	}

	if (currentPage)
	{
		PositionInputData pos = getInputDataRelativeToCurrentPage(currentPage, event);
		currentPage->onButtonReleaseEvent(pos);
	}

	//Reset the selection
	EditSelection* tmpSelection = xournal->selection;
	xournal->selection = nullptr;

	toolHandler->restoreLastConfig();

	// we need this workaround so it's possible to select something with the middle button
	if (tmpSelection)
	{
		xournal->view->setSelection(tmpSelection);
	}

	this->inputRunning = false;
	this->lastHitEvent = nullptr;

	return false;
}

void PenInputHandler::actionLeaveWindow(GdkEvent* event)
{
	if (!this->penInWidget)
	{
		return;
	}

	this->penInWidget = false;

	// Stop input sequence if the tool is not a selection tool
	ToolHandler* toolHandler = this->inputContext->getToolHandler();
	ToolType toolType = toolHandler->getToolType();
	if (this->inputRunning && toolType != TOOL_SELECT_OBJECT && toolType != TOOL_SELECT_RECT && toolType != TOOL_SELECT_REGION)
	{
		this->actionEnd(this->lastHitEvent);
	} else if (this->deviceClassPressed)
	{
		// scroll if we have an active selection
		gdouble eventX, eventY;
		gdk_event_get_coords(event, &eventX, &eventY);

		GtkWidget* widget = this->inputContext->getView()->getWidget();
		gint width = gtk_widget_get_allocated_width(widget);
		gint height = gtk_widget_get_allocated_height(widget);

		new std::thread([&]()
		{
			int offsetX = 0, offsetY = 0;

			// TODO: make offset dependent on how big the distance between pen and view is
			if (eventX < 25)
			{
				offsetX = -10;
			}

			if (eventY < 25)
			{
				offsetY = -10;
			}

			if (eventX > width - 25)
			{
				offsetX = 10;
			}

			if (eventY > height - 25)
			{
				offsetY = 10;
			}

#ifdef DEBUG_INPUT
			g_message("Offset: X:%d\tY:%d", offsetX, offsetY);
#endif

			while (!this->penInWidget)
			{
				Util::execInUiThread([&]()
				{
					GtkXournal* xournal = this->inputContext->getXournal();
					xournal->layout->scrollRelative(offsetX, offsetY);
				});

				//sleep for half a second until we scroll again
				g_usleep((gulong) (0.5 * G_USEC_PER_SEC));
			}
		});

	}
}

void PenInputHandler::actionEnterWindow(GdkEvent* event)
{
	this->penInWidget = true;

	// Restart input sequence if the tool is pressed and not a selection tool
	ToolHandler* toolHandler = this->inputContext->getToolHandler();
	ToolType toolType = toolHandler->getToolType();
	// TODO should we use the event state to determine the pressed state of the device? This currently disallows strokes starting outside of the widget
	if (this->deviceClassPressed && toolType != TOOL_SELECT_OBJECT && toolType != TOOL_SELECT_RECT && toolType != TOOL_SELECT_REGION)
	{
		this->actionStart(event);
	}
}
