//
// Created by ulrich on 17.05.19.
//

#include "InputEvents.h"

InputEvent::~InputEvent()
{
	gdk_event_free(this->sourceEvent);
}

InputEvent* InputEvent::copy()
{
	auto inputEvent = new InputEvent();

	inputEvent->sourceEvent = gdk_event_copy(this->sourceEvent);
	inputEvent->type = this->type;
	inputEvent->deviceClass = this->deviceClass;
	inputEvent->deviceName = this->deviceName;

	inputEvent->absoluteX = this->absoluteX;
	inputEvent->absoluteY = this->absoluteY;
	inputEvent->relativeX = this->relativeX;
	inputEvent->relativeY = this->relativeY;

	inputEvent->button = this->button;
	inputEvent->state = this->state;
	inputEvent->pressure = this->pressure;

	inputEvent->sequence = this->sequence;
	inputEvent->timestamp = this->timestamp;

	return inputEvent;
}

InputEventType InputEvents::translateEventType(GdkEventType type)
{
	switch (type)
	{
		case GDK_MOTION_NOTIFY:
		case GDK_TOUCH_UPDATE:
			return MOTION_EVENT;
		case GDK_BUTTON_PRESS:
		case GDK_TOUCH_BEGIN:
			return BUTTON_PRESS_EVENT;
		case GDK_2BUTTON_PRESS:
			return BUTTON_2_PRESS_EVENT;
		case GDK_BUTTON_RELEASE:
		case GDK_TOUCH_END:
		case GDK_TOUCH_CANCEL:
			return BUTTON_RELEASE_EVENT;
		case GDK_ENTER_NOTIFY:
			return ENTER_EVENT;
		case GDK_LEAVE_NOTIFY:
			return LEAVE_EVENT;
		case GDK_PROXIMITY_IN:
			return PROXIMITY_IN_EVENT;
		case GDK_PROXIMITY_OUT:
			return PROXIMITY_OUT_EVENT;
		case GDK_SCROLL:
			return SCROLL_EVENT;
		case GDK_GRAB_BROKEN:
			return GRAB_BROKEN_EVENT;
		case GDK_KEY_PRESS:
			return KEY_PRESS_EVENT;
		case GDK_KEY_RELEASE:
			return KEY_RELEASE_EVENT;
		default:
			// Events we do not care about
			return UNKNOWN;
	}
}

InputDeviceClass InputEvents::translateDeviceType(GdkDevice* device, Settings* settings)
{
	int deviceType = settings->getDeviceClassForDevice(device);
	switch (deviceType)
	{
		case 0:
		{
			// Keyboards are not matched in their own class - do this here manually
			if (gdk_device_get_source(device) == GDK_SOURCE_KEYBOARD)
			{
				return INPUT_DEVICE_KEYBOARD;
			}
			return INPUT_DEVICE_IGNORE;
		}
		case 1:
			return INPUT_DEVICE_MOUSE;
		case 2:
			return INPUT_DEVICE_PEN;
		case 3:
			return INPUT_DEVICE_ERASER;
		case 4:
			return INPUT_DEVICE_TOUCHSCREEN;
		default:
			return INPUT_DEVICE_IGNORE;
	}
}

InputEvent* InputEvents::translateEvent(GdkEvent* sourceEvent, Settings* settings)
{
	auto targetEvent = new InputEvent();

	targetEvent->sourceEvent = sourceEvent;

	// Map the event type to our internal ones
	GdkEventType sourceEventType = gdk_event_get_event_type(sourceEvent);
	targetEvent->type = translateEventType(sourceEventType);

	GdkDevice* device = gdk_event_get_source_device(sourceEvent);
	targetEvent->deviceClass = translateDeviceType(device, settings);

	targetEvent->deviceName = const_cast<gchar*>(gdk_device_get_name(device));

	// Copy both coordinates of the event
	gdk_event_get_root_coords(sourceEvent, &(targetEvent->absoluteX), &(targetEvent->absoluteY));
	gdk_event_get_coords(sourceEvent, &(targetEvent->relativeX), &(targetEvent->relativeY));

	// Copy the event button if there is any
	if (targetEvent->type == BUTTON_PRESS_EVENT || targetEvent->type == BUTTON_RELEASE_EVENT)
	{
		gdk_event_get_button(sourceEvent, &(targetEvent->button));
	}
	if (sourceEventType & (GDK_TOUCH_BEGIN | GDK_TOUCH_END | GDK_TOUCH_CANCEL))
	{
		// As we only handle single finger events we can set the button statically to 1
		targetEvent->button = 1;
	}
	gdk_event_get_state(sourceEvent, &targetEvent->state);
	if (targetEvent->deviceClass == INPUT_DEVICE_KEYBOARD)
	{
		gdk_event_get_keyval(sourceEvent, &targetEvent->button);
	}

	// Copy the event sequence if there is any
	if (sourceEventType & (GDK_TOUCH_BEGIN | GDK_TOUCH_UPDATE | GDK_TOUCH_END | GDK_TOUCH_CANCEL))
	{
		targetEvent->sequence = gdk_event_get_event_sequence(sourceEvent);
	}

	// Copy the timestamp
	targetEvent->timestamp = gdk_event_get_time(sourceEvent);

	//Copy the pressure data
	gdk_event_get_axis(sourceEvent, GDK_AXIS_PRESSURE, &targetEvent->pressure);

	return targetEvent;
}
