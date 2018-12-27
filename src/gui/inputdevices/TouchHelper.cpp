#include "TouchHelper.h"

#include "control/settings/Settings.h"


TouchHelper::TouchHelper(Settings* settings)
 : settings(settings),
   enabled(false),
   touchState(true),
   disableTimeout(500)
{
	XOJ_INIT_TYPE(TouchHelper);

	reload();
}

TouchHelper::~TouchHelper()
{
	XOJ_CHECK_TYPE(TouchHelper);

	// Enable touchscreen on quit application
	if (!touchState)
	{
		enableTouch();
	}

	XOJ_RELEASE_TYPE(TouchHelper);
}

/**
 * Reload settings
 */
void TouchHelper::reload()
{
	XOJ_CHECK_TYPE(TouchHelper);

	SElement& touch = settings->getCustomElement("touch");

	enabled = false;
	touch.getBool("disableTouch", enabled);

	disableTimeout = 500;
	touch.getInt("timeout", disableTimeout);
	if (disableTimeout < 500)
	{
		disableTimeout = 500;
	}


//	string disableMethod;
//	touch.getString("method", disableMethod);
//	int methodId = 0;
//	if (disableMethod == "X11")
//	{
//		methodId = 1;
//	}
//	else if (disableMethod == "custom")
//	{
//		methodId = 2;
//	}
//
//	gtk_combo_box_set_active(GTK_COMBO_BOX(get("cbTouchDisableMethod")), methodId);
//
//	string cmd;
//	touch.getString("cmdEnable", cmd);
//	gtk_entry_set_text(GTK_ENTRY(get("txtEnableTouchCommand")), cmd.c_str());
//
//	cmd = "";
//	touch.getString("cmdDisable", cmd);
//	gtk_entry_set_text(GTK_ENTRY(get("txtDisableTouchCommand")), cmd.c_str());

}

/**
 * Called after the timeout
 *
 * @return true to call again
 */
bool TouchHelper::enableTimeout(TouchHelper* self)
{
	XOJ_CHECK_TYPE_OBJ(self, TouchHelper);

	gint64 now = g_get_monotonic_time() / 1000;
	gint64 lastPenActionTime = now - self->lastPenAction;
	if (lastPenActionTime < 100)
	{
		// Pen action within the last 100ms, so simple restart timeout
		return true;
	}

	if (lastPenActionTime > self->disableTimeout)
	{
		// Timeout elapsed, enable touch again
		self->enableTouch();
		self->touchState = true;

		// Do not call again
		return false;
	}

	int nextTime = now - self->lastPenAction + self->disableTimeout;

	g_timeout_add(nextTime, (GSourceFunc)enableTimeout, self);

	// Do not call again, a new time is scheduled
	return false;
}

/**
 * There was a pen event, restart the timer
 */
void TouchHelper::penEvent()
{
	XOJ_CHECK_TYPE(TouchHelper);
	lastPenAction = g_get_monotonic_time() / 1000;

	if (touchState)
	{
		touchState = false;
		disableTouch();
		g_timeout_add(disableTimeout, (GSourceFunc)enableTimeout, this);
	}
}

/**
 * Enable touchscreen
 */
void TouchHelper::enableTouch()
{
	printf("enable touch\n");
}

/**
 * Disable touchscreen
 */
void TouchHelper::disableTouch()
{
	printf("disable touch\n");

}

/**
 * An event from a device occurred
 */
void TouchHelper::event(GdkDevice* device)
{
	XOJ_CHECK_TYPE(TouchHelper);

	if (!enabled)
	{
		return;
	}

	GdkInputSource dev = gdk_device_get_source(device);

	if (dev == GDK_SOURCE_PEN || dev == GDK_SOURCE_ERASER)
	{
		penEvent();
	}
}


