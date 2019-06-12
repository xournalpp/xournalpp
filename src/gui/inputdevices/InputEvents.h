/*
 * Xournal++
 *
 * [Header description]
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <glib.h>
#include <gdk/gdk.h>
#include <control/settings/Settings.h>

enum InputEventType
{
	UNKNOWN,
	BUTTON_PRESS_EVENT,
	BUTTON_2_PRESS_EVENT,
	BUTTON_3_PRESS_EVENT,
	BUTTON_RELEASE_EVENT,
	MOTION_EVENT,
	ENTER_EVENT,
	LEAVE_EVENT,
	PROXIMITY_IN_EVENT,
	PROXIMITY_OUT_EVENT,
	SCROLL_EVENT,
	GRAB_BROKEN_EVENT,
	KEY_PRESS_EVENT,
	KEY_RELEASE_EVENT
};

enum InputDeviceClass
{
	INPUT_DEVICE_MOUSE,
	INPUT_DEVICE_PEN,
	INPUT_DEVICE_ERASER,
	INPUT_DEVICE_TOUCHSCREEN,
	INPUT_DEVICE_KEYBOARD,
	INPUT_DEVICE_IGNORE
};

class InputEvent
{
public:
	GdkEvent* sourceEvent = nullptr;

	InputEventType type = UNKNOWN;

	InputDeviceClass deviceClass = INPUT_DEVICE_IGNORE;
	gchar* deviceName = nullptr;


	gdouble absoluteX = 0;
	gdouble absoluteY = 0;
	gdouble relativeX = 0;
	gdouble relativeY = 0;

	guint button = 0;
	GdkModifierType state = (GdkModifierType) 0;
	gdouble pressure = 1.0;

	GdkEventSequence* sequence = nullptr;
	guint32 timestamp = 0;

	~InputEvent();

	InputEvent* copy();
};

class InputEvents
{

	static InputEventType translateEventType(GdkEventType type);

public:

	static InputDeviceClass translateDeviceType(GdkDevice* device, Settings* settings);
	static InputDeviceClass translateDeviceType(const string& name, GdkInputSource source, Settings* settings);

	static InputEvent* translateEvent(GdkEvent* sourceEvent, Settings* settings);
};


