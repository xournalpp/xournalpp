#include "AbstractInputDevice.h"

#include "control/Control.h"
#include "control/settings/ButtonConfig.h"
#include "gui/Cursor.h"
#include "gui/Layout.h"
#include "gui/pageposition/PagePositionHandler.h"
#include "gui/widgets/XournalWidget.h"
#include "gui/XournalView.h"


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
}

XojPageView* gtk_xournal_get_page_view_for_pos_cached(GtkXournal* xournal, int x, int y)
{
	x += xournal->x;
	y += xournal->y;

	PagePositionHandler* pph = xournal->view->getPagePositionHandler();

	return pph->getViewAt(x, y, xournal->pagePositionCache);
}


AbstractInputDevice::AbstractInputDevice(GtkWidget* widget, XournalView* view)
 : widget(widget),
   view(view),
   current_view(NULL)
{
	XOJ_INIT_TYPE(AbstractInputDevice);
}

AbstractInputDevice::~AbstractInputDevice()
{
	XOJ_CHECK_TYPE(AbstractInputDevice);

	widget = NULL;
	view = NULL;

	XOJ_RELEASE_TYPE(AbstractInputDevice);
}

bool AbstractInputDevice::handleButtonPress(GdkEventButton* event)
{
	XOJ_CHECK_TYPE(AbstractInputDevice);

	GtkXournal* xournal = GTK_XOURNAL(widget);

	gtk_widget_grab_focus(widget);

	// none button release event was sent, send one now
	if (xournal->currentInputPage)
	{
		GdkEventButton ev = *event;
		xournal->currentInputPage->translateEvent((GdkEvent*) &ev, xournal->x, xournal->y);
		xournal->currentInputPage->onButtonReleaseEvent(widget, &ev);
	}

	ToolHandler* h = xournal->view->getControl()->getToolHandler();

	// Change the tool depending on the key or device
	if (changeTool(event->device, event->button))
	{
		return true;
	}

	// hand tool don't change the selection, so you can scroll e.g.
	// with your touchscreen without remove the selection
	if (h->getToolType() == TOOL_HAND)
	{
		Cursor* cursor = xournal->view->getCursor();
		cursor->setMouseDown(true);
		xournal->inScrolling = true;
		//set reference
		xournal->lastMousePositionX = event->x_root;
		xournal->lastMousePositionY = event->y_root;

		return TRUE;
	}
	else if (xournal->selection)
	{
		EditSelection* selection = xournal->selection;

		XojPageView* view = selection->getView();
		GdkEventButton ev = *event;
		view->translateEvent((GdkEvent*) &ev, xournal->x, xournal->y);
		CursorSelectionType selType = selection->getSelectionTypeForPos(ev.x, ev.y, xournal->view->getZoom());
		if (selType)
		{

			if (selType == CURSOR_SELECTION_MOVE && event->button == 3)
			{
				selection->copySelection();
			}

			xournal->view->getCursor()->setMouseDown(true);
			xournal->selection->mouseDown(selType, ev.x, ev.y);
			return true;
		}
		else
		{
			xournal->view->clearSelection();

			if (changeTool(event->device, event->button))
			{
				return true;
			}
		}
	}

	XojPageView* pv = gtk_xournal_get_page_view_for_pos_cached(xournal, event->x, event->y);

	current_view = pv;

	if (pv)
	{
		xournal->currentInputPage = pv;
		pv->translateEvent((GdkEvent*) event, xournal->x, xournal->y);

		xournal->view->getDocument()->indexOf(pv->getPage());
		return pv->onButtonPressEvent(widget, event);
	}

	return FALSE; // not handled
}

bool AbstractInputDevice::handleButtonRelease(GdkEventButton* event)
{
	XOJ_CHECK_TYPE(AbstractInputDevice);

	current_view = NULL;

	GtkXournal* xournal = GTK_XOURNAL(widget);

	Cursor* cursor = xournal->view->getCursor();
	ToolHandler* h = xournal->view->getControl()->getToolHandler();
	if (xournal->view->zoom_gesture_active)
	{
		return true;
	}

	cursor->setMouseDown(false);

	xournal->inScrolling = false;

	EditSelection* sel = xournal->view->getSelection();
	if (sel)
	{
		sel->mouseUp();
	}

	bool res = false;
	if (xournal->currentInputPage)
	{
		xournal->currentInputPage->translateEvent((GdkEvent*) event, xournal->x, xournal->y);
		res = xournal->currentInputPage->onButtonReleaseEvent(widget, event);
		xournal->currentInputPage = NULL;
	}

	EditSelection* tmpSelection = xournal->selection;
	xournal->selection = NULL;

	h->restoreLastConfig();

	// we need this workaround so it's possible to select something with the middle button
	if (tmpSelection)
	{
		xournal->view->setSelection(tmpSelection);
	}

	return res;
}

bool AbstractInputDevice::handleMotion(GdkEventMotion* event)
{
	GtkXournal* xournal = GTK_XOURNAL(widget);

	ToolHandler* h = xournal->view->getControl()->getToolHandler();

	if (xournal->view->zoom_gesture_active)
	{
		return true;
	}

	if (h->getToolType() == TOOL_HAND)
	{
		if (xournal->inScrolling)
		{
			gtk_xournal_scroll_mouse_event(xournal, event);
			return TRUE;
		}
		return FALSE;
	}
	else if (xournal->selection)
	{
		EditSelection* selection = xournal->selection;

		XojPageView* view = selection->getView();
		GdkEventMotion ev = *event;
		view->translateEvent((GdkEvent*) &ev, xournal->x, xournal->y);

		if (xournal->selection->isMoving())
		{
			selection->mouseMove(ev.x, ev.y);
		}
		else
		{
			CursorSelectionType selType = selection->getSelectionTypeForPos(ev.x, ev.y, xournal->view->getZoom());
			xournal->view->getCursor()->setMouseSelectionType(selType);
		}
		return true;
	}

	XojPageView* pv = NULL;

	if (current_view)
	{
		pv = current_view;
	}
	else
	{
		pv = gtk_xournal_get_page_view_for_pos_cached(xournal, event->x, event->y);
	}

	xournal->view->getCursor()->setInsidePage(pv != NULL);

	if (pv)
	{
		// allow events only to a single page!
		if (xournal->currentInputPage == NULL || pv == xournal->currentInputPage)
		{
			return motionEvent(pv, event);
		}
	}

	return false;
}

bool AbstractInputDevice::changeTool(GdkDevice* device, int button)
{
	Settings* settings = view->getControl()->getSettings();
	ButtonConfig* cfgTouch = settings->getTouchButtonConfig();
	ToolHandler* h = view->getControl()->getToolHandler();

	GtkXournal* xournal = GTK_XOURNAL(widget);
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
	 else if (button == 2)   // Middle Button
	{
		cfg = settings->getMiddleButtonConfig();
	}
	else if (button == 3 && !xournal->selection)     // Right Button
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

	return false;
}



