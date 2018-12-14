#include "DirectAxisInputDevice.h"

#include "control/Control.h"
#include "gui/XournalView.h"

#include <cmath>

DirectAxisInputDevice::DirectAxisInputDevice(GtkWidget* widget, XournalView* view)
 : BaseInputDevice(widget, view)
{
	XOJ_INIT_TYPE(DirectAxisInputDevice);
}

DirectAxisInputDevice::~DirectAxisInputDevice()
{
	XOJ_RELEASE_TYPE(DirectAxisInputDevice);
}

/**
 * Read Pressure over GTK
 */
bool DirectAxisInputDevice::getPressureMultiplier(GdkEvent* event, double& pressure)
{
	XOJ_CHECK_TYPE(DirectAxisInputDevice);

	GdkDevice* device = gdk_event_get_device(event);
	int axesCount = gdk_device_get_n_axes(device);
	if (axesCount <= 4)
	{
		return false;
	}

	double* axes;
	if (event->type == GDK_MOTION_NOTIFY)
	{
		axes = event->motion.axes;
	}
	else
	{
		axes = event->button.axes;
	}

	pressure = axes[2];
	Settings* settings = view->getControl()->getSettings();

	if (!finite(pressure))
	{
		return false;
	}

	pressure = ((1 - pressure) * settings->getWidthMinimumMultiplier()
			+ pressure * settings->getWidthMaximumMultiplier());

	return true;
}
