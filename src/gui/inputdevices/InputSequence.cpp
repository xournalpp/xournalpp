#include "InputSequence.h"
#include "NewGtkInputDevice.h"

#include "control/settings/ButtonConfig.h"
#include "control/settings/Settings.h"
#include "control/tools/EditSelection.h"
#include "control/ToolHandler.h"
#include "gui/Cursor.h"
#include "gui/pageposition/PagePositionHandler.h"
#include "gui/PageView.h"
#include "gui/XournalView.h"



/*
bool gtk_xournal_scroll_callback(GtkXournal* xournal)
{
	xournal->layout->scrollRelativ(xournal->scrollOffsetX, xournal->scrollOffsetY);

	// Scrolling done, so reset our counters
	xournal->scrollOffsetX = 0;
	xournal->scrollOffsetY = 0;

	return false;
}

static void gtk_xournal_scroll_mouse_event(GtkXournal* xournal, GdkEventMotion* event)
{
	// use root coordinates as reference point because
	// scrolling changes window relative coordinates
	// see github Gnome/evince@1adce5486b10e763bed869
	int x_root = event->x_root;
	int y_root = event->y_root;

	if (xournal->lastMousePositionX - x_root == 0 && xournal->lastMousePositionY - y_root == 0)
	{
		return;
	}

	if (xournal->scrollOffsetX == 0 && xournal->scrollOffsetY == 0)
	{
		xournal->scrollOffsetX = xournal->lastMousePositionX - x_root;
		xournal->scrollOffsetY = xournal->lastMousePositionY - y_root;

		g_idle_add((GSourceFunc) gtk_xournal_scroll_callback, xournal);
		//gtk_xournal_scroll_callback(xournal);
		xournal->lastMousePositionX = x_root;
		xournal->lastMousePositionY = y_root;
	}
}*/



InputSequence::InputSequence(NewGtkInputDevice* inputHandler)
 : inputRunning(false),
   inputHandler(inputHandler),
   current_view(NULL),
   currentInputPage(NULL),
   device(NULL),
   axes(NULL),
   button(0),
   x(-1),
   y(-1)
{
	XOJ_INIT_TYPE(InputSequence);

	this->presureSensitivity = inputHandler->getSettings()->isPresureSensitivity();
}

InputSequence::~InputSequence()
{
	XOJ_CHECK_TYPE(InputSequence);

	clearAxes();

	XOJ_RELEASE_TYPE(InputSequence);
}

/**
 * Set current input device
 */
void InputSequence::setDevice(GdkDevice* device)
{
	XOJ_CHECK_TYPE(InputSequence);

	this->device = device;
}

/**
 * Clear the last stored axes
 */
void InputSequence::clearAxes()
{
	XOJ_CHECK_TYPE(InputSequence);

	g_clear_pointer(&axes, g_free);
}

/**
 * Set the axes
 *
 * @param axes Will be handed over, and freed by InputSequence
 */
void InputSequence::setAxes(gdouble* axes)
{
	XOJ_CHECK_TYPE(InputSequence);

	clearAxes();
	this->axes = axes;
}

/**
 * Copy axes from event
 */
void InputSequence::copyAxes(GdkEvent* event)
{
	XOJ_CHECK_TYPE(InputSequence);

	clearAxes();
	setAxes((gdouble*)g_memdup(event->motion.axes, sizeof(gdouble) * gdk_device_get_n_axes(device)));
}

/**
 * Set Position
 */
void InputSequence::setCurrentPosition(double x, double y)
{
	XOJ_CHECK_TYPE(InputSequence);

	this->x = x;
	this->y = y;
}

/**
 * Set (mouse)button
 */
void InputSequence::setButton(guint button)
{
	XOJ_CHECK_TYPE(InputSequence);

	this->button = button;
}

/**
 * Get Page at current position
 *
 * @return page or NULL if none
 */
XojPageView* InputSequence::getPageAtCurrentPosition()
{
	XOJ_CHECK_TYPE(InputSequence);

	GtkXournal* xournal = inputHandler->getXournal();

	double x = this->x + xournal->x;
	double y = this->y + xournal->y;

	PagePositionHandler* pph = xournal->view->getPagePositionHandler();

	return pph->getViewAt(x, y, xournal->pagePositionCache);
}

/**
 * Mouse / Pen / Touch move
 */
bool InputSequence::actionMoved()
{
	XOJ_CHECK_TYPE(InputSequence);

	GtkXournal* xournal = inputHandler->getXournal();
	ToolHandler* h = inputHandler->getToolHandler();

	printf("moved %s %i (%ld)\n", gdk_device_get_name(device), inputRunning, this);


	changeTool();

	if (xournal->view->zoom_gesture_active)
	{
		return false;
	}

	if (h->getToolType() == TOOL_HAND)
	{
		if (xournal->inScrolling)
		{
			// TODO gtk_xournal_scroll_mouse_event(xournal, event);
			return true;
		}
		return false;
	}
	else if (xournal->selection)
	{
//		EditSelection* selection = xournal->selection;
//
//		XojPageView* view = selection->getView();
//		GdkEventMotion ev = *event;
//		view->translateEvent((GdkEvent*) &ev, xournal->x, xournal->y);
//
//		if (xournal->selection->isMoving())
//		{
//			selection->mouseMove(ev.x, ev.y);
//		}
//		else
//		{
//			CursorSelectionType selType = selection->getSelectionTypeForPos(ev.x, ev.y, xournal->view->getZoom());
//			xournal->view->getCursor()->setMouseSelectionType(selType);
//		}
//		return true;
	}

	XojPageView* pv = NULL;

	if (current_view)
	{
		pv = current_view;
	}
	else
	{
		pv = getPageAtCurrentPosition();
	}

	xournal->view->getCursor()->setInsidePage(pv != NULL);

	if (pv && inputRunning)
	{
		// allow events only to a single page!
		if (currentInputPage == NULL || pv == currentInputPage)
		{
			PositionInputData pos = getInputDataRelativeToCurrentPage(pv);
			return pv->onMotionNotifyEvent(pos, xournal->shiftDown);
		}
	}

	return false;
}

/**
 * Mouse / Pen down / touch start
 */
bool InputSequence::actionStart()
{
	XOJ_CHECK_TYPE(InputSequence);

	inputHandler->focusWidget();

	checkCanStartInput();

	if (!inputRunning)
	{
		return false;
	}

	// none button release event was sent, send one now
	// only for this device, other devices may still have unfinished input
	if (currentInputPage)
	{
		PositionInputData pos = getInputDataRelativeToCurrentPage(currentInputPage);
		currentInputPage->onButtonReleaseEvent(pos);
	}

	ToolHandler* h = inputHandler->getToolHandler();

	// Change the tool depending on the key or device
	if (changeTool())
	{
		return true;
	}

	GtkXournal* xournal = inputHandler->getXournal();

	// hand tool don't change the selection, so you can scroll e.g.
	// with your touchscreen without remove the selection
	if (h->getToolType() == TOOL_HAND)
	{
//		Cursor* cursor = xournal->view->getCursor();
//		cursor->setMouseDown(true);
//		xournal->inScrolling = true;
//		//set reference
//		xournal->lastMousePositionX = event->x_root;
//		xournal->lastMousePositionY = event->y_root;
//
//		return TRUE;
	}
	else if (xournal->selection)
	{
//		EditSelection* selection = xournal->selection;
//
//		XojPageView* view = selection->getView();
//		GdkEventButton ev = *event;
//		view->translateEvent((GdkEvent*) &ev, xournal->x, xournal->y);
//		CursorSelectionType selType = selection->getSelectionTypeForPos(ev.x, ev.y, xournal->view->getZoom());
//		if (selType)
//		{
//
//			if (selType == CURSOR_SELECTION_MOVE && event->button == 3)
//			{
//				selection->copySelection();
//			}
//
//			xournal->view->getCursor()->setMouseDown(true);
//			xournal->selection->mouseDown(selType, ev.x, ev.y);
//			return true;
//		}
//		else
//		{
//			xournal->view->clearSelection();
//			if (changeTool(event))
//			{
//				return true;
//			}
//		}
	}

	XojPageView* pv = getPageAtCurrentPosition();

	current_view = pv;

	if (pv)
	{
		currentInputPage = pv;

		PositionInputData pos = getInputDataRelativeToCurrentPage(pv);
		return pv->onButtonPressEvent(pos);
	}

	// not handled
	return false;
}

/**
 * Mouse / Pen up / touch end
 */
void InputSequence::actionEnd()
{
	XOJ_CHECK_TYPE(InputSequence);

	if (!inputRunning)
	{
		return;
	}

	printf("InputSequence::actionEnd\n");

	current_view = NULL;

	GtkXournal* xournal = inputHandler->getXournal();
	Cursor* cursor = xournal->view->getCursor();
	ToolHandler* h = inputHandler->getToolHandler();

	if (xournal->view->zoom_gesture_active)
	{
		stopInput();
		return;
	}

	cursor->setMouseDown(false);

	xournal->inScrolling = false;

	EditSelection* sel = xournal->view->getSelection();
	if (sel)
	{
		sel->mouseUp();
	}

	if (currentInputPage)
	{
		PositionInputData pos = getInputDataRelativeToCurrentPage(currentInputPage);
		currentInputPage->onButtonReleaseEvent(pos);
		currentInputPage = NULL;
	}

	EditSelection* tmpSelection = xournal->selection;
	xournal->selection = NULL;

	h->restoreLastConfig();

	// we need this workaround so it's possible to select something with the middle button
	if (tmpSelection)
	{
		xournal->view->setSelection(tmpSelection);
	}

	stopInput();
}

/**
 * Get input data relative to current input page
 */
PositionInputData InputSequence::getInputDataRelativeToCurrentPage(XojPageView* page)
{
	XOJ_CHECK_TYPE(InputSequence);

	GtkXournal* xournal = inputHandler->getXournal();

	PositionInputData pos;
	pos.x = x - page->getX() - xournal->x;
	pos.y = y - page->getY() - xournal->y;
	pos.pressure = 1.0;

	if (presureSensitivity)
	{
		gdk_device_get_axis(device, axes, GDK_AXIS_PRESSURE, &pos.pressure);
	}

	// TODO Key event
	pos.state = 0;

	return pos;
}


/**
 * Check if this input can be started (don't do two inputs at the same time)
 */
void InputSequence::checkCanStartInput()
{
	XOJ_CHECK_TYPE(InputSequence);

	if (inputHandler->startInput(this))
	{
		printf("start input %s\n", gdk_device_get_name(device));
		inputRunning = true;
	}
	else
	{
		printf("NOT start input %s\n", gdk_device_get_name(device));
		inputRunning = false;
	}
}

/**
 * Stop the running input, if running
 */
void InputSequence::stopInput()
{
	XOJ_CHECK_TYPE(InputSequence);

	if (inputRunning)
	{
		printf("stop input %s\n", gdk_device_get_name(device));
	}

	inputRunning = false;
	inputHandler->stopInput(this);
}

/**
 * Change the tool according to the device and button
 * @return true to ignore event
 */
bool InputSequence::changeTool()
{
	XOJ_CHECK_TYPE(InputSequence);

	Settings* settings = inputHandler->getSettings();
	ButtonConfig* cfgTouch = settings->getTouchButtonConfig();
	ToolHandler* h = inputHandler->getToolHandler();
	GtkXournal* xournal = inputHandler->getXournal();

	ButtonConfig* cfg = NULL;
	if (gdk_device_get_source(device) == GDK_SOURCE_PEN)
	{
		if (button == 2)
		{
			cfg = settings->getStylusButton1Config();
		}
		else if (button == 3)
		{
			cfg = settings->getStylusButton2Config();
		}
	}
	else if (button == 2 /* Middle Button */)
	{
		cfg = settings->getMiddleButtonConfig();
	}
	else if (button == 3 /* Right Button */ && !xournal->selection)
	{
		cfg = settings->getRightButtonConfig();
	}
	else if (gdk_device_get_source(device) == GDK_SOURCE_ERASER)
	{
		cfg = settings->getEraserButtonConfig();
	}
	else if (cfgTouch->device == gdk_device_get_name(device))
	{
		cfg = cfgTouch;

		// If an action is defined we do it, even if it's a drawing action...
		if (cfg->getDisableDrawing() && cfg->getAction() == TOOL_NONE)
		{
			ToolType tool = h->getToolType();
			if (tool == TOOL_PEN || tool == TOOL_ERASER || tool == TOOL_HILIGHTER)
			{
				printf("ignore touchscreen for drawing!\n");
				return true;
			}
		}
	}

	if (cfg && cfg->getAction() != TOOL_NONE)
	{
		h->copyCurrentConfig();
		cfg->acceptActions(h);
	}
	else
	{
		h->restoreLastConfig();
	}

	return false;
}

/**
 * Free an input sequence, used as callback for GTK
 */
void InputSequence::free(InputSequence* sequence)
{
	delete sequence;
}
