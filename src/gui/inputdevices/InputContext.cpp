//
// Created by ulrich on 06.04.19.
//

#include "InputContext.h"
#include "InputEvents.h"

#include <util/DeviceListHelper.h>

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

	for (const InputDevice& savedDevices: this->view->getControl()->getSettings()->getKnownInputDevices()) {
		this->knownDevices.insert(savedDevices.getName());
	}
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
			GDK_TOUCH_MASK |
			GDK_POINTER_MOTION_MASK |
			GDK_BUTTON_PRESS_MASK |
			GDK_BUTTON_RELEASE_MASK |
			GDK_SMOOTH_SCROLL_MASK |
			GDK_ENTER_NOTIFY_MASK |
			GDK_LEAVE_NOTIFY_MASK |
			GDK_PROXIMITY_IN_MASK |
			GDK_PROXIMITY_OUT_MASK;

	gtk_widget_add_events(pWidget, mask);

	g_signal_connect(pWidget, "event", G_CALLBACK(eventCallback), this);
}

bool InputContext::eventCallback(GtkWidget* widget, GdkEvent* event, InputContext* self)
{
	return self->handle(event);
}

bool InputContext::handle(GdkEvent* sourceEvent)
{
	XOJ_CHECK_TYPE(InputContext);

	printDebug(sourceEvent);

	InputEvent* event = InputEvents::translateEvent(sourceEvent, this->getSettings());

	// Add the device to the list of known devices if it is currently unknown
	GdkDevice* sourceDevice = gdk_event_get_source_device(sourceEvent);
	if (GDK_SOURCE_KEYBOARD != gdk_device_get_source(sourceDevice) &&
	    gdk_device_get_device_type(sourceDevice) != GDK_DEVICE_TYPE_MASTER &&
	    gdk_device_get_vendor_id(sourceDevice) != nullptr && gdk_device_get_product_id(sourceDevice) != nullptr &&
	    this->knownDevices.find(string(event->deviceName)) == this->knownDevices.end()) {
		this->knownDevices.insert(string(event->deviceName));
		this->getSettings()->transactionStart();
		this->getSettings()->setDeviceClassForDevice(gdk_event_get_source_device(sourceEvent), event->deviceClass);
		this->getSettings()->transactionEnd();
	}

	// We do not handle scroll events manually but let GTK do it for us
	if (event->type == SCROLL_EVENT)
	{
		// Hand over to standard GTK Scroll / Zoom handling
		return false;
	}

	// Deactivate touchscreen when a pen event occurs
	this->getView()->getHandRecognition()->event(event->deviceClass);

	// Get the state of all modifiers
	this->modifierState = event->state;

	// separate events to appropriate handlers
	// handle tablet stylus
	if (event->deviceClass == INPUT_DEVICE_PEN || event->deviceClass == INPUT_DEVICE_ERASER)
	{
		return this->stylusHandler->handle(event);
	}

	// handle mouse devices
	if (event->deviceClass == INPUT_DEVICE_MOUSE)
	{
		return this->mouseHandler->handle(event);
	}

	// handle touchscreens
	if (event->deviceClass == INPUT_DEVICE_TOUCHSCREEN)
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
	if (event->deviceClass == INPUT_DEVICE_KEYBOARD)
	{
		return this->keyboardHandler->handle(event);
	}

	if (event->deviceClass == INPUT_DEVICE_IGNORE)
	{
		return true;
	}

	delete event;

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

void InputContext::printDebug(GdkEvent* event)
{
	XOJ_CHECK_TYPE(InputContext);

#ifdef DEBUG_INPUT_GDK_PRINT_EVENTS
	gdk_set_show_events(true);
#else
#ifdef DEBUG_INPUT
	string message = "Event\n";
	string gdkEventTypes[] = {
			"GDK_NOTHING", "GDK_DELETE", "GDK_DESTROY", "GDK_EXPOSE", "GDK_MOTION_NOTIFY", "GDK_BUTTON_PRESS", "GDK_DOUBLE_BUTTON_PRESS",
			"GDK_TRIPLE_BUTTON_PRESS", "GDK_BUTTON_RELEASE", "GDK_KEY_PRESS", "GDK_KEY_RELEASE", "GDK_ENTER_NOTIFY", "GDK_LEAVE_NOTIFY", "GDK_FOCUS_CHANGE",
			"GDK_CONFIGURE", "GDK_MAP", "GDK_UNMAP", "GDK_PROPERTY_NOTIFY", "GDK_SELECTION_CLEAR", "GDK_SELECTION_REQUEST", "GDK_SELECTION_NOTIFY",
			"GDK_PROXIMITY_IN", "GDK_PROXIMITY_OUT", "GDK_DRAG_ENTER", "GDK_DRAG_LEAVE", "GDK_DRAG_MOTION", "GDK_DRAG_STATUS", "GDK_DROP_START",
			"GDK_DROP_FINISHED", "GDK_CLIENT_EVENT", "GDK_VISIBILITY_NOTIFY", "", "GDK_SCROLL", "GDK_WINDOW_STATE", "GDK_SETTING", "GDK_OWNER_CHANGE",
			"GDK_GRAB_BROKEN", "GDK_DAMAGE", "GDK_TOUCH_BEGIN", "GDK_TOUCH_UPDATE", "GDK_TOUCH_END", "GDK_TOUCH_CANCEL", "GDK_TOUCHPAD_SWIPE",
			"GDK_TOUCHPAD_PINCH", "GDK_PAD_BUTTON_PRESS", "GDK_PAD_BUTTON_RELEASE", "GDK_PAD_RING", "GDK_PAD_STRIP", "GDK_PAD_GROUP_MODE", "GDK_EVENT_LAST"
	};
	message += "Event type:\t" + gdkEventTypes[event->type + 1] + "\n";

	string gdkInputSources[] = {
			"GDK_SOURCE_MOUSE",	"GDK_SOURCE_PEN", "GDK_SOURCE_ERASER", "GDK_SOURCE_CURSOR", "GDK_SOURCE_KEYBOARD", "GDK_SOURCE_TOUCHSCREEN", "GDK_SOURCE_TOUCHPAD",
			"GDK_SOURCE_TRACKPOINT", "GDK_SOURCE_TABLET_PAD"
	};
	GdkDevice* device = gdk_event_get_source_device(event);
	message += "Source device:\t" + gdkInputSources[gdk_device_get_source(device)] + "\n";
	string gdkInputClasses[] = {
			"INPUT_DEVICE_MOUSE", "INPUT_DEVICE_PEN", "INPUT_DEVICE_ERASER", "INPUT_DEVICE_TOUCHSCREEN", "INPUT_DEVICE_KEYBOARD", "INPUT_DEVICE_IGNORE"
	};
	InputDeviceClass deviceClass = InputEvents::translateDeviceType(device, this->getSettings());
	message += "Device Class:\t" + gdkInputClasses[deviceClass] + "\n";

	if (event->type == GDK_BUTTON_PRESS || event->type == GDK_DOUBLE_BUTTON_PRESS || event->type == GDK_TRIPLE_BUTTON_PRESS || event->type == GDK_BUTTON_RELEASE)
	{
		guint button;
		if (gdk_event_get_button(event, &button))
		{
			message += "Button:\t" + std::to_string(button) + "\n";
		}
	}

#ifndef DEBUG_INPUT_PRINT_ALL_MOTION_EVENTS
	static bool motionEventBlock = false;
	if (event->type == GDK_MOTION_NOTIFY)
	{
		if (!motionEventBlock)
		{
			motionEventBlock = true;
			g_message("%s", message.c_str());
		}
	}
	else
	{
		motionEventBlock = false;
		g_message("%s", message.c_str());
	}
#else
	g_message("%s", message.c_str());
#endif //DEBUG_INPUT_PRINT_ALL_MOTION_EVENTS
#endif //DEBUG_INPUT
#endif //DEBUG_INPUT_PRINT_EVENTS
}
