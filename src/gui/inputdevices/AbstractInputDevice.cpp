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
   view(view)
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




