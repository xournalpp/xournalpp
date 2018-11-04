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

gboolean XInputUtils::onMouseEnterNotifyEvent(GtkWidget* widget, GdkEventCrossing* event)
{
#if !GTK3_ENABLED
	if (!XInputUtils::enableLeafEnterWorkaround)
	{
		return FALSE;
	}

	INPUTDBG("enter notify\n", NULL);

	/* re-enable input devices after they've been emergency-disabled
	 by leave_notify */
	if (!gtk_check_version(2, 17, 0))
	{
		gdk_flush();
		gdk_error_trap_push();
		for (GList* dev_list = gdk_devices_list(); dev_list != NULL; dev_list = dev_list->next)
		{
			GdkDevice* dev = GDK_DEVICE(dev_list->data);
			gdk_device_set_mode(dev, GDK_MODE_SCREEN);
		}
		gdk_flush();
		gdk_error_trap_pop();
	}

#endif
	return FALSE;
}

gboolean XInputUtils::onMouseLeaveNotifyEvent(GtkWidget* widget, GdkEventCrossing* event)
{
#if !GTK3_ENABLED

	if (!XInputUtils::enableLeafEnterWorkaround)
	{
		return FALSE;
	}

	INPUTDBG("leave notify (mode=%d, details=%d)\n", event->mode, event->detail);

	/* emergency disable XInput to avoid segfaults (GTK+ 2.17) or
	 interface non-responsiveness (GTK+ 2.18) */
	if (!gtk_check_version(2, 17, 0))
	{
		gdk_flush();
		gdk_error_trap_push();
		for (GList* dev_list = gdk_devices_list(); dev_list != NULL;
			dev_list = dev_list->next)
		{
			GdkDevice* dev = GDK_DEVICE(dev_list->data);
			gdk_device_set_mode(dev, GDK_MODE_DISABLED);
		}
		gdk_flush();
		gdk_error_trap_pop();
	}

#endif
	return FALSE;
}
