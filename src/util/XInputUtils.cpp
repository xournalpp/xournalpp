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

void XInputUtils::fixXInputCoords(GdkEvent* event, GtkWidget* widget)
{
	double* axes, *px, *py;
	GdkDevice* device;
	int wx, wy, ix, iy;

	if (event->type == GDK_BUTTON_PRESS || event->type == GDK_BUTTON_RELEASE)
	{
		axes = event->button.axes;
		px = &(event->button.x);
		py = &(event->button.y);
		device = event->button.device;
	}
	else if (event->type == GDK_MOTION_NOTIFY)
	{
		axes = event->motion.axes;
		px = &(event->motion.x);
		py = &(event->motion.y);
		device = event->motion.device;
	}
	else
	{
		return; // nothing we know how to do
	}

	if (axes == NULL)
	{
		return;
	}

#ifdef ENABLE_XINPUT_BUGFIX
	// fix broken events with the core pointer's location
	if (!finite(axes[0]) || !finite(axes[1]) || (axes[0] == 0. && axes[1] == 0.))
	{
		gdk_window_get_pointer(GTK_WIDGET(widget)->window, &ix, &iy, NULL);
		*px = ix;
		*py = iy;
	}
	else
	{
		gdk_window_get_origin(GTK_WIDGET(widget)->window, &wx, &wy);
		double axis_width = device->axes[0].max - device->axes[0].min;
		if (axis_width > EPSILON)
		{
			*px = (axes[0] / axis_width) * screenWidth - wx;
		}
		axis_width = device->axes[1].max - device->axes[1].min;
		if (axis_width > EPSILON)
		{
			*py = (axes[1] / axis_width) * screenHeight - wy;
		}
	}
#else
	if (!finite(*px) || !finite(*py) || (*px == 0. && *py == 0.))
	{
		gdk_window_get_pointer(GTK_WIDGET(widget)->window, &ix, &iy, NULL);
		*px = ix;
		*py = iy;
	}
	else
	{
		/* with GTK+ 2.17, events come improperly translated, and the event's
		 GdkWindow isn't even the same for ButtonDown as for MotionNotify... */
		if (gtk_major_version == 2 && gtk_minor_version == 17) // GTK+ 2.17 issues !!
		{
			gdk_window_get_position(GTK_WIDGET(widget)->window, &wx, &wy);
			*px += wx;
			*py += wy;
		}
	}
#endif
}

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
	return FALSE;
}

gboolean XInputUtils::onMouseLeaveNotifyEvent(GtkWidget* widget, GdkEventCrossing* event)
{
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
	return FALSE;
}
