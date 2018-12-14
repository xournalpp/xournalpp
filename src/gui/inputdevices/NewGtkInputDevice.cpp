#include "NewGtkInputDevice.h"

#include "control/Control.h"
#include "gui/PageView.h"
#include "gui/widgets/XournalWidget.h"
#include "gui/XournalView.h"
#include "model/Point.h"


NewGtkInputDevice::NewGtkInputDevice(GtkWidget* widget, XournalView* view)
 : AbstractInputDevice(widget, view)
{
	XOJ_INIT_TYPE(NewGtkInputDevice);
}

NewGtkInputDevice::~NewGtkInputDevice()
{
	XOJ_RELEASE_TYPE(NewGtkInputDevice);
}

/**
 * Initialize the input handling, set input events
 */
void NewGtkInputDevice::initWidget()
{
	XOJ_CHECK_TYPE(NewGtkInputDevice);

	gtk_widget_set_support_multidevice(widget, true);
	gtk_widget_add_events(widget,
			// Key handling
			GDK_KEY_PRESS_MASK |

			// Touch / Pen / Mouse
			GDK_POINTER_MOTION_MASK |
			GDK_BUTTON_PRESS_MASK |
			GDK_BUTTON_RELEASE_MASK |
			GDK_SMOOTH_SCROLL_MASK |
			GDK_ENTER_NOTIFY_MASK |
			GDK_LEAVE_NOTIFY_MASK |
			GDK_TOUCH_MASK);

    g_signal_connect(widget, "event", G_CALLBACK(event_cb), this);
}

bool NewGtkInputDevice::event_cb(GtkWidget* widget, GdkEvent* event, NewGtkInputDevice* self)
{
	XOJ_CHECK_TYPE_OBJ(self, NewGtkInputDevice);

	return self->eventHandler(event);
}

/**
 * Handle all GTK Events
 */
bool NewGtkInputDevice::eventHandler(GdkEvent* event)
{

	return false;
}

/**
 * Mouse / pen moved event
 */
bool NewGtkInputDevice::motionEvent(XojPageView* pageView, GdkEventMotion* event)
{
	XOJ_CHECK_TYPE(NewGtkInputDevice);

/*
	gdouble x, y;
	AxesInfo *info;

	GdkDevice* device = gdk_event_get_device(event);
	GdkDevice* source_device = gdk_event_get_source_device(event);
	GdkEventSequence* sequence = gdk_event_get_event_sequence(event);
	GdkDeviceTool* tool = gdk_event_get_device_tool(event);

	if (event->type == GDK_TOUCH_END || event->type == GDK_TOUCH_CANCEL)
	{
		g_hash_table_remove(data->touch_info, sequence);
		return true;
	}
	else if (event->type == GDK_LEAVE_NOTIFY)
	{
		g_hash_table_remove(data->pointer_info, device);
		return;
	}

	if (!sequence)
	{
		info = g_hash_table_lookup(data->pointer_info, device);

		if (!info)
		{
			info = axes_info_new();
			g_hash_table_insert(data->pointer_info, device, info);
		}
	}
	else
	{
		info = g_hash_table_lookup(data->touch_info, sequence);

		if (!info)
		{
			info = axes_info_new();
			g_hash_table_insert(data->touch_info, sequence, info);
		}
	}

	if (info->last_source != source_device)
		info->last_source = source_device;

	if (info->last_tool != tool)
		info->last_tool = tool;

	g_clear_pointer(&info->axes, g_free);

	if (event->type == GDK_TOUCH_BEGIN || event->type == GDK_TOUCH_UPDATE)
	{
		if (sequence && event->touch.emulating_pointer)
			g_hash_table_remove(data->pointer_info, device);
	}
	if (event->type == GDK_MOTION_NOTIFY)
	{
		info->axes = g_memdup(event->motion.axes, sizeof(gdouble) * gdk_device_get_n_axes(source_device));
	}
	else if (event->type == GDK_BUTTON_PRESS || event->type == GDK_BUTTON_RELEASE)
	{
		info->axes = g_memdup(event->button.axes, sizeof(gdouble) * gdk_device_get_n_axes(source_device));
	}

	if (gdk_event_get_coords(event, &x, &y))
	{
		info->x = x;
		info->y = y;
	}
*/
	return false;
}

/**
 * Read pressure and position of the pen, if a pen is active
 */
void NewGtkInputDevice::readPositionAndPressure(GdkEventMotion* event, double& x, double& y, double& pressure)
{
	XOJ_CHECK_TYPE(NewGtkInputDevice);

	x = event->x;
	y = event->y;

	bool presureSensitivity = view->getControl()->getSettings()->isPresureSensitivity();

	if (presureSensitivity)
	{
		return;
	}

	ToolHandler* h = view->getControl()->getToolHandler();

	if (h->getToolType() != TOOL_PEN)
	{
		return;
	}

	if (!getPressureMultiplier((GdkEvent*) event, pressure))
	{
		pressure = Point::NO_PRESURE;
	}
}

/**
 * Read Pressure over GTK
 */
bool NewGtkInputDevice::getPressureMultiplier(GdkEvent* event, double& pressure)
{
	XOJ_CHECK_TYPE(NewGtkInputDevice);

	return true;
}


