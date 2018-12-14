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

	int events = GDK_EXPOSURE_MASK;
	events |= GDK_POINTER_MOTION_MASK;
	events |= GDK_EXPOSURE_MASK;
	events |= GDK_BUTTON_MOTION_MASK;

	// See documentation: https://developer.gnome.org/gtk3/stable/chap-input-handling.html
	events |= GDK_TOUCH_MASK;
	events |= GDK_BUTTON_PRESS_MASK;
	events |= GDK_BUTTON_RELEASE_MASK;
	events |= GDK_ENTER_NOTIFY_MASK;
	events |= GDK_LEAVE_NOTIFY_MASK;
	events |= GDK_KEY_PRESS_MASK;
	events |= GDK_SCROLL_MASK;

	// NOT Working with GTK3, only with GTK2
	// Therefore listening for mouse move events
	// of the pen, and add a timeout
	//	events |= GDK_PROXIMITY_IN_MASK;
	//	events |= GDK_PROXIMITY_OUT_MASK;

	gtk_widget_set_events(widget, events);
}

/**
 * Mouse / pen moved event
 */
bool NewGtkInputDevice::motionEvent(XojPageView* pageView, GdkEventMotion* event)
{
	XOJ_CHECK_TYPE(NewGtkInputDevice);

	double x = 0;
	double y = 0;
	double pressure = Point::NO_PRESURE;
	readPositionAndPressure(event, x, y, pressure);

	GtkXournal* xournal = GTK_XOURNAL(widget);

	double pageX = x - pageView->getX() - xournal->x;
	double pageY = y - pageView->getY() - xournal->y;

	return pageView->onMotionNotifyEvent(widget, pageX, pageY, pressure, xournal->shiftDown);
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

/**
 * Touch event
 */
bool NewGtkInputDevice::touchEvent(GdkEventTouch* event)
{
	XOJ_CHECK_TYPE(NewGtkInputDevice);


	// This handler consume some touch events
	// Not fully working, but fixes some touch
	// issues

	// Consume event
	return true;
}

