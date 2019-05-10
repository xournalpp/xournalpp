//
// Created by ulrich on 06.04.19.
//

#include "PenInputHandler.h"
#include "AbstractInputHandler.h"
#include "InputContext.h"

#include <gui/widgets/XournalWidget.h>
#include <control/ToolHandler.h>
#include <gui/XournalppCursor.h>
#include <gui/inputdevices/PositionInputData.h>
#include "gui/XournalView.h"
#include <control/settings/ButtonConfig.h>

#include <cmath>

#define WIDGET_SCROLL_BORDER 25
#define INPUT_STOP_DRAWING_ON_LEAVE false

PenInputHandler::PenInputHandler(InputContext* inputContext) : AbstractInputHandler(inputContext)
{
	XOJ_INIT_TYPE(PenInputHandler);
}

PenInputHandler::~PenInputHandler()
{
	XOJ_CHECK_TYPE(PenInputHandler);

	XOJ_RELEASE_TYPE(PenInputHandler);
}

void PenInputHandler::updateLastEvent(GdkEvent* event)
{
	XOJ_CHECK_TYPE(PenInputHandler);

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

void PenInputHandler::handleScrollEvent(GdkEvent* event)
{
	XOJ_CHECK_TYPE(PenInputHandler);

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
	XOJ_CHECK_TYPE(PenInputHandler);

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
	ToolType toolType = toolHandler->getToolType();

	// Save the starting offset when hand-tool is selected to get a reference for the scroll-offset
	if (toolType == TOOL_HAND)
	{
		gdk_event_get_root_coords(event, &this->scrollStartX, &this->scrollStartY);
	}

	// Set the reference page for selections and other single-page elements so motion events are passed to the right page everytime
	if (toolHandler->isSinglePageTool())
	{
		this->sequenceStartPage = currentPage;
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
	XOJ_CHECK_TYPE(PenInputHandler);

	/*
	 * Workaround for misbehaving devices where Enter events are not published every time
	 * This is required to disable outside scrolling again
	 */
	gdouble eventX, eventY;
	if (gdk_event_get_coords(event, &eventX, &eventY))
	{
		GtkAdjustment* adjHorizontal = this->inputContext->getScrollHandling()->getHorizontal();
		GtkAdjustment* adjVertical = this->inputContext->getScrollHandling()->getVertical();
		double h = gtk_adjustment_get_value(adjHorizontal);
		double v = gtk_adjustment_get_value(adjVertical);
		eventX -= h;
		eventY -= v;

		GtkWidget* widget = gtk_widget_get_parent(this->inputContext->getView()->getWidget());
		gint width = gtk_widget_get_allocated_width(widget);
		gint height = gtk_widget_get_allocated_height(widget);


		if (!this->penInWidget && eventX > WIDGET_SCROLL_BORDER && eventY > WIDGET_SCROLL_BORDER && eventX < width - WIDGET_SCROLL_BORDER && eventY < height - WIDGET_SCROLL_BORDER)
		{
			this->penInWidget = true;
		}
	}



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

	ToolHandler* toolHandler = this->inputContext->getToolHandler();
	if (!toolHandler->isSinglePageTool())
	{
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

			bool result = this->actionStart(event);
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
			bool result = this->actionStart(event);
			this->updateLastEvent(event);
			return result;
		}
	}

	// Update the last position of the input device
	this->updateLastEvent(event);

	// Update the cursor
	xournal->view->getCursor()->setInsidePage(currentPage != nullptr);

	// Selections and single-page elements will always work on one page so we need to handle them differently
	if (this->sequenceStartPage && toolHandler->isSinglePageTool())
	{
		// Relay the event to the page
		PositionInputData pos = getInputDataRelativeToCurrentPage(sequenceStartPage, event);

		// Enforce input to stay within page
		pos.x = std::max(0.0, pos.x);
		pos.y = std::max(0.0, pos.y);
		pos.x = std::min(pos.x, (double) sequenceStartPage->getDisplayWidth());
		pos.y = std::min(pos.y, (double) sequenceStartPage->getDisplayHeight());

		return sequenceStartPage->onMotionNotifyEvent(pos);
	}

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
	XOJ_CHECK_TYPE(PenInputHandler);

	GtkXournal* xournal = inputContext->getXournal();
	XournalppCursor* cursor = xournal->view->getCursor();
	ToolHandler* toolHandler = inputContext->getToolHandler();

	cursor->setMouseDown(false);

	EditSelection* sel = xournal->view->getSelection();
	if (sel)
	{
		sel->mouseUp();
	}

	// Selections and single-page elements will always work on one page so we need to handle them differently
	if (this->sequenceStartPage && toolHandler->isSinglePageTool())
	{
		PositionInputData pos = getInputDataRelativeToCurrentPage(this->sequenceStartPage, event);
		this->sequenceStartPage->onButtonReleaseEvent(pos);
	} else
	{
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
	}

	//Reset the selection
	EditSelection* tmpSelection = xournal->selection;
	xournal->selection = nullptr;
	this->sequenceStartPage = nullptr;

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
	XOJ_CHECK_TYPE(PenInputHandler);

	if (!this->penInWidget)
	{
		return;
	}

	if (!this->inputContext->getSettings()->getInputSystemDrawOutsideWindowEnabled())
	{
		this->penInWidget = false;
	}

	// Stop input sequence if the tool is not a selection tool
	ToolHandler* toolHandler = this->inputContext->getToolHandler();
	if (this->inputRunning && !toolHandler->isSinglePageTool())
	{
		if (!this->inputContext->getSettings()->getInputSystemDrawOutsideWindowEnabled())
		{
			this->actionEnd(this->lastHitEvent);
		}
	} else if (this->deviceClassPressed)
	{
		// scroll if we have an active selection
		gdouble eventX, eventY;
		if (!gdk_event_get_coords(event, &eventX, &eventY))
		{
			g_warning("PenInputHandler: LeaveWindow: Could not get coordinates!");
			return;
		}

		GtkAdjustment* adjHorizontal = this->inputContext->getScrollHandling()->getHorizontal();
		GtkAdjustment* adjVertical = this->inputContext->getScrollHandling()->getVertical();
		double h = gtk_adjustment_get_value(adjHorizontal);
		double v = gtk_adjustment_get_value(adjVertical);
		eventX -= h;
		eventY -= v;

		GtkWidget* widget = gtk_widget_get_parent(this->inputContext->getView()->getWidget());
		gint width = gtk_widget_get_allocated_width(widget);
		gint height = gtk_widget_get_allocated_height(widget);

		new std::thread([&,eventX,eventY, width, height]()
		{
			int offsetX = 0, offsetY = 0;

			// TODO: make offset dependent on how big the distance between pen and view is
			if (eventX < WIDGET_SCROLL_BORDER)
			{
				offsetX = -10;
			}

			if (eventY < WIDGET_SCROLL_BORDER)
			{
				offsetY = -10;
			}

			if (eventX > width - WIDGET_SCROLL_BORDER)
			{
				offsetX = 10;
			}

			if (eventY > height - WIDGET_SCROLL_BORDER)
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
	XOJ_CHECK_TYPE(PenInputHandler);

	this->penInWidget = true;

	if (!this->inputContext->getSettings()->getInputSystemDrawOutsideWindowEnabled())
	{
		// Restart input sequence if the tool is pressed and not a single-page tool
		ToolHandler* toolHandler = this->inputContext->getToolHandler();
		if (this->deviceClassPressed && !toolHandler->isSinglePageTool())
		{
			this->actionStart(event);
		}
	}
}
