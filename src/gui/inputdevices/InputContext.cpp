//
// Created by ulrich on 06.04.19.
//

#include "InputContext.h"

InputContext::InputContext(XournalView* view, ScrollHandling* scrollHandling)
{
	XOJ_INIT_TYPE(InputContext);

	this->view = view;
	this->scrollHandling = scrollHandling;

	this->stylusHandler = new StylusInputHandler(this);
	this->touchHandler = new TouchInputHandler(this);
	this->touchDrawingHandler = new TouchDrawingInputHandler(this);
	this->mouseHandler = new MouseInputHandler(this);
	this->keyboardHandler = new KeyboardInputHandler(this);

	this->touchWorkaroundEnabled = this->getSettings()->isTouchWorkaround();
}

InputContext::~InputContext()
{
	XOJ_CHECK_TYPE(InputContext);

	delete this->stylusHandler;
	this->stylusHandler = nullptr;

	delete this->touchHandler;
	this->touchHandler = nullptr;

	delete this->touchDrawingHandler;
	this->touchDrawingHandler = nullptr;

	delete this->mouseHandler;
	this->mouseHandler = nullptr;
	
	delete this->keyboardHandler;
	this->keyboardHandler = nullptr;

	XOJ_RELEASE_TYPE(InputContext);
}

void InputContext::connect(GtkWidget* pWidget)
{
	XOJ_CHECK_TYPE(InputContext);

	this->widget = pWidget;
	gtk_widget_set_support_multidevice(widget, true);

	int mask =
			// Key handling
			GDK_KEY_PRESS_MASK |

			// Allow scrolling
			GDK_SCROLL_MASK |

			// Touch / Pen / Mouse
			GDK_TOUCH_MASK          |
			GDK_POINTER_MOTION_MASK |
			GDK_BUTTON_PRESS_MASK   |
			GDK_BUTTON_RELEASE_MASK |
			GDK_SMOOTH_SCROLL_MASK  |
			GDK_ENTER_NOTIFY_MASK   |
			GDK_LEAVE_NOTIFY_MASK	|
			GDK_PROXIMITY_IN_MASK	|
			GDK_PROXIMITY_OUT_MASK;

	gtk_widget_add_events(pWidget, mask);

	g_signal_connect(pWidget, "event", G_CALLBACK(eventCallback), this);
}

bool InputContext::eventCallback(GtkWidget* widget, GdkEvent* event, InputContext* self)
{
	return self->handle(event);
}

bool InputContext::handle(GdkEvent* event)
{
	XOJ_CHECK_TYPE(InputContext);

	GdkDevice* device = gdk_event_get_source_device(event);

	// We do not handle scroll events manually but let GTK do it for us
	if (event->type == GDK_SCROLL)
	{
		// Hand over to standard GTK Scroll / Zoom handling
		return false;
	}

	// Deactivate touchscreen when a pen event occurs
	this->getView()->getHandRecognition()->event(device);

	// Get the state of all modifiers
	auto state = (GdkModifierType)0;
	if (gdk_event_get_state(event, &state))
	{
		this->modifierState = state;
	}

	// separate events to appropriate handlers
	// handle tablet stylus
	if (gdk_device_get_source(device) == GDK_SOURCE_PEN || gdk_device_get_source(device) == GDK_SOURCE_ERASER)
	{
		return this->stylusHandler->handle(event);
	}

	// handle mouse devices
#if (GTK_MAJOR_VERSION >= 3 && GTK_MINOR_VERSION >= 22)
	if (gdk_device_get_source(device) == GDK_SOURCE_MOUSE || gdk_device_get_source(device) == GDK_SOURCE_TOUCHPAD || gdk_device_get_source(device) == GDK_SOURCE_TRACKPOINT)
#else
	if (gdk_device_get_source(device) == GDK_SOURCE_MOUSE || gdk_device_get_source(device) == GDK_SOURCE_TOUCHPAD)
#endif
	{
		return this->mouseHandler->handle(event);
	}

	// handle touchscreens
	if (gdk_device_get_source(device) == GDK_SOURCE_TOUCHSCREEN)
	{
		// trigger touch drawing depending on the setting
		if (this->touchWorkaroundEnabled)
		{
			return this->touchDrawingHandler->handle(event);
		} else
		{
			return this->touchHandler->handle(event);
		}
	}

	// handle keyboard
	if (gdk_device_get_source(device) == GDK_SOURCE_KEYBOARD)
	{
		return this->keyboardHandler->handle(event);
	}

	//We received an event we do not have a handler for
	return false;
}

GtkXournal* InputContext::getXournal()
{
	XOJ_CHECK_TYPE(InputContext);

	return GTK_XOURNAL(widget);
}

XournalView* InputContext::getView()
{
	XOJ_CHECK_TYPE(InputContext);

	return view;
}

Settings* InputContext::getSettings()
{
	XOJ_CHECK_TYPE(InputContext);

	return view->getControl()->getSettings();
}

ToolHandler* InputContext::getToolHandler()
{
	XOJ_CHECK_TYPE(InputContext);

	return view->getControl()->getToolHandler();
}

ScrollHandling* InputContext::getScrollHandling()
{
	XOJ_CHECK_TYPE(InputContext);

	return this->scrollHandling;
}

GdkModifierType InputContext::getModifierState()
{
	XOJ_CHECK_TYPE(InputContext);

	return this->modifierState;
}

/**
 * Focus the widget
 */
void InputContext::focusWidget()
{
	XOJ_CHECK_TYPE(InputContext);

	if (!gtk_widget_has_focus(widget))
	{
		gtk_widget_grab_focus(widget);
	}
}

void InputContext::blockDevice(InputContext::DeviceType deviceType)
{
	XOJ_CHECK_TYPE(InputContext);

	switch (deviceType)
	{
		case MOUSE:
			this->mouseHandler->block(true);
			break;
		case STYLUS:
			this->stylusHandler->block(true);
			break;
		case TOUCHSCREEN:
			this->touchDrawingHandler->block(true);
			this->touchHandler->block(true);
			this->getView()->getZoomGestureHandler()->disable();
			break;
	}
}

void InputContext::unblockDevice(InputContext::DeviceType deviceType)
{
	XOJ_CHECK_TYPE(InputContext);

	switch (deviceType)
	{
		case MOUSE:
			this->mouseHandler->block(false);
			break;
		case STYLUS:
			this->stylusHandler->block(false);
			break;
		case TOUCHSCREEN:
			this->touchDrawingHandler->block(false);
			this->touchHandler->block(false);
			this->getView()->getZoomGestureHandler()->enable();
			break;
	}
}

bool InputContext::isBlocked(InputContext::DeviceType deviceType)
{
	XOJ_CHECK_TYPE(InputContext);

	switch (deviceType)
	{
		case MOUSE:
			return this->mouseHandler->isBlocked();
		case STYLUS:
			return this->stylusHandler->isBlocked();
		case TOUCHSCREEN:
			return this->touchDrawingHandler->isBlocked();
	}
	return false;
}
