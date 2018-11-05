#include "XInputUtils.h"

#include <config-dev.h>

#include <math.h>

int XInputUtils::screenWidth = 0;
int XInputUtils::screenHeight = 0;
int XInputUtils::enableLeafEnterWorkaround = true;

XInputUtils::XInputUtils()
{
}

XInputUtils::~XInputUtils()
{
}

void XInputUtils::initUtils(GtkWidget* win)
{
	GdkScreen* screen = gtk_widget_get_screen(win);
	XInputUtils::screenWidth = gdk_screen_get_width(screen);
	XInputUtils::screenHeight = gdk_screen_get_height(screen);
}

void XInputUtils::setLeafEnterWorkaroundEnabled(bool enabled)
{
	if (XInputUtils::enableLeafEnterWorkaround == true && enabled == false)
	{
		XInputUtils::onMouseEnterNotifyEvent(NULL, NULL);
	}

	XInputUtils::enableLeafEnterWorkaround = enabled;
}

#define EPSILON 1E-7

void XInputUtils::handleScrollEvent(GdkEventButton* event, GtkWidget* widget)
{
	GdkEvent scrollEvent;
	/* with GTK+ 2.17 and later, the entire widget hierarchy is xinput-aware,
	 so the core button event gets discarded and the scroll event never
	 gets processed by the main window. This is arguably a GTK+ bug.
	 We work around it. */

	scrollEvent.scroll.type = GDK_SCROLL;
	scrollEvent.scroll.window = event->window;
	scrollEvent.scroll.send_event = event->send_event;
	scrollEvent.scroll.time = event->time;
	scrollEvent.scroll.x = event->x;
	scrollEvent.scroll.y = event->y;
	scrollEvent.scroll.state = event->state;
	scrollEvent.scroll.device = event->device;
	scrollEvent.scroll.x_root = event->x_root;
	scrollEvent.scroll.y_root = event->y_root;
	if (event->button == 4)
	{
		scrollEvent.scroll.direction = GDK_SCROLL_UP;
	}
	else if (event->button == 5)
	{
		scrollEvent.scroll.direction = GDK_SCROLL_DOWN;
	}
	else if (event->button == 6)
	{
		scrollEvent.scroll.direction = GDK_SCROLL_LEFT;
	}
	else
	{
		scrollEvent.scroll.direction = GDK_SCROLL_RIGHT;
	}
	gtk_widget_event(widget, &scrollEvent);
}

// TODO Remove
gboolean XInputUtils::onMouseEnterNotifyEvent(GtkWidget* widget, GdkEventCrossing* event)
{
	return FALSE;
}

// TODO Remove
gboolean XInputUtils::onMouseLeaveNotifyEvent(GtkWidget* widget, GdkEventCrossing* event)
{
	return FALSE;
}
