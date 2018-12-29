#include "TouchHelper.h"
#include "touchdisable/TouchDisableCustom.h"
#include "touchdisable/TouchDisableX11.h"

#include "control/settings/Settings.h"


TouchHelper::TouchHelper(Settings* settings)
 : enabled(false),
   touchState(true),
   disableTimeout(500),
   x11Session(false),
   touchImpl(NULL),
   settings(settings)
{
	XOJ_INIT_TYPE(TouchHelper);

	reload();

#ifdef X11_ENABLED
	const char* sessionType = g_getenv("XDG_SESSION_TYPE");
	if (sessionType != NULL && strcmp(sessionType, "x11") == 0)
	{
		x11Session = true;
	}
#endif
}

TouchHelper::~TouchHelper()
{
	XOJ_CHECK_TYPE(TouchHelper);

	// Enable touchscreen on quit application
	if (!touchState)
	{
		enableTouch();
	}

	delete touchImpl;
	touchImpl = NULL;

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

	disableTimeout = 1000;
	touch.getInt("timeout", disableTimeout);
	if (disableTimeout < 500)
	{
		disableTimeout = 500;
	}

	delete touchImpl;
	touchImpl = NULL;

	string disableMethod;
	touch.getString("method", disableMethod);
	if (disableMethod == "X11")
	{
		if (x11Session == false)
		{
			g_warning("X11 Touch workaround is selected, but no X11 Session running!");
			enabled = false;
			return;
		}
#ifdef X11_ENABLED
		touchImpl = new TouchDisableX11();
#endif
	}
	else if (disableMethod == "custom")
	{
		string enableCommand;
		touch.getString("cmdEnable", enableCommand);
		string disableCommand;
		touch.getString("cmdDisable", disableCommand);

		touchImpl = new TouchDisableCustom(enableCommand, disableCommand);
	}
	else // Auto detect
	{
#ifdef X11_ENABLED
		if (x11Session)
		{
			touchImpl = new TouchDisableX11();
		}
#endif
	}

	if (touchImpl)
	{
		touchImpl->init();
	}
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
	if (touchImpl)
	{
		touchImpl->enableTouch();
	}
}

/**
 * Disable touchscreen
 */
void TouchHelper::disableTouch()
{
	if (touchImpl)
	{
		touchImpl->disableTouch();
	}
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


