#include "NewGtkInputDevice.h"
#include "InputSequence.h"

#include "control/Control.h"
#include "gui/PageView.h"
#include "gui/widgets/XournalWidget.h"
#include "gui/XournalView.h"
#include "model/Point.h"

NewGtkInputDevice::NewGtkInputDevice(GtkWidget* widget, XournalView* view)
 : AbstractInputDevice(widget, view)
{
	XOJ_INIT_TYPE(NewGtkInputDevice);

	pointerInputList = g_hash_table_new_full(NULL, NULL, NULL, (GDestroyNotify) InputSequence::free);
	touchInputList = g_hash_table_new_full(NULL, NULL, NULL, (GDestroyNotify) InputSequence::free);
}

NewGtkInputDevice::~NewGtkInputDevice()
{
	XOJ_CHECK_TYPE(NewGtkInputDevice);

	g_hash_table_destroy(pointerInputList);
	g_hash_table_destroy(touchInputList);

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

	GdkDevice* device = gdk_event_get_device(event);
	GdkDevice* sourceDevice = gdk_event_get_source_device(event);
	GdkEventSequence* sequence = gdk_event_get_event_sequence(event);

	if (event->type == GDK_TOUCH_END || event->type == GDK_TOUCH_CANCEL)
	{
		InputSequence* input = (InputSequence*) g_hash_table_lookup(touchInputList, sequence);

		if (input != NULL)
		{
			input->actionEnd();
		}

		g_hash_table_remove(touchInputList, sequence);
		return true;
	}
	else if (event->type == GDK_LEAVE_NOTIFY)
	{
		g_hash_table_remove(pointerInputList, device);
		return true;
	}

	InputSequence* input = NULL;
	if (sequence == NULL)
	{
		input = (InputSequence*) g_hash_table_lookup(pointerInputList, device);

		if (input == NULL)
		{
			input = new InputSequence();
			g_hash_table_insert(pointerInputList, device, input);
		}
	}
	else
	{
		input = (InputSequence*) g_hash_table_lookup(touchInputList, sequence);

		if (input == NULL)
		{
			input = new InputSequence();
			g_hash_table_insert(touchInputList, sequence, input);
		}
	}

	// Apply the correct device if not yet set, should not change
	// But GTK decides which inputs are get together
	input->setDevice(sourceDevice);
	input->clearAxes();

	if (event->type == GDK_TOUCH_BEGIN)
	{
		input->actionStart();
	}

	if (event->type == GDK_TOUCH_BEGIN || event->type == GDK_TOUCH_UPDATE)
	{
		if (sequence && event->touch.emulating_pointer)
		{
			g_hash_table_remove(pointerInputList, device);
			printf("Touch emulating pointer\n");
			return true;
		}
	}


	gdouble x, y;
	if (gdk_event_get_coords(event, &x, &y))
	{
		input->setCurrentPosition(x, y);
	}

	if (event->type == GDK_MOTION_NOTIFY || event->type == GDK_TOUCH_UPDATE)
	{
		input->copyAxes(event);
		input->actionMoved();
	}
	else if (event->type == GDK_BUTTON_PRESS)
	{
		input->copyAxes(event);
		input->actionStart();
	}
	else if (event->type == GDK_BUTTON_RELEASE)
	{
		input->copyAxes(event);
		input->actionEnd();
	}

	return true;
}

/**
 * Mouse / pen moved event
 */
bool NewGtkInputDevice::motionEvent(XojPageView* pageView, GdkEventMotion* event)
{
	XOJ_CHECK_TYPE(NewGtkInputDevice);

	return true;
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


