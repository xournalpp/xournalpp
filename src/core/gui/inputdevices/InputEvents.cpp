//
// Created by ulrich on 17.05.19.
//

#include "InputEvents.h"

#include "control/settings/Settings.h"       // for Settings
#include "control/settings/SettingsEnums.h"  // for InputDeviceTypeOption

auto InputEvents::translateEventType(GdkEventType type) -> InputEventType {
    switch (type) {
        case GDK_MOTION_NOTIFY:
        case GDK_TOUCH_UPDATE:
            return MOTION_EVENT;
        case GDK_BUTTON_PRESS:
        case GDK_TOUCH_BEGIN:
            return BUTTON_PRESS_EVENT;
        case GDK_2BUTTON_PRESS:
            return BUTTON_2_PRESS_EVENT;
        case GDK_3BUTTON_PRESS:
            return BUTTON_3_PRESS_EVENT;
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

auto InputEvents::translateDeviceType(const std::string& name, GdkInputSource source, Settings* settings)
        -> InputDeviceClass {
    InputDeviceTypeOption deviceType = settings->getDeviceClassForDevice(name, source);
    switch (deviceType) {
        case InputDeviceTypeOption::Disabled: {
            // Keyboards are not matched in their own class - do this here manually
            if (source == GDK_SOURCE_KEYBOARD) {
                return INPUT_DEVICE_KEYBOARD;
            }
            return INPUT_DEVICE_IGNORE;
        }
        case InputDeviceTypeOption::Mouse:
            return INPUT_DEVICE_MOUSE;
        case InputDeviceTypeOption::Pen:
            return INPUT_DEVICE_PEN;
        case InputDeviceTypeOption::Eraser:
            return INPUT_DEVICE_ERASER;
        case InputDeviceTypeOption::Touchscreen:
            return INPUT_DEVICE_TOUCHSCREEN;
        case InputDeviceTypeOption::MouseKeyboardCombo:
            return INPUT_DEVICE_MOUSE_KEYBOARD_COMBO;
        default:
            return INPUT_DEVICE_IGNORE;
    }
}

auto InputEvents::translateDeviceType(GdkDevice* device, Settings* settings) -> InputDeviceClass {
    return translateDeviceType(gdk_device_get_name(device), gdk_device_get_source(device), settings);
}

auto InputEvents::translateEvent(GdkEvent* sourceEvent, Settings* settings) -> InputEvent {
    InputEvent targetEvent{};

    targetEvent.sourceEvent = sourceEvent;

    // Map the event type to our internal ones
    GdkEventType sourceEventType = gdk_event_get_event_type(sourceEvent);
    targetEvent.type = translateEventType(sourceEventType);

    GdkDevice* device = gdk_event_get_source_device(sourceEvent);
    targetEvent.deviceClass = translateDeviceType(device, settings);

    targetEvent.deviceName = const_cast<gchar*>(gdk_device_get_name(device));

    // Copy both coordinates of the event
    gdk_event_get_root_coords(sourceEvent, &targetEvent.absoluteX, &targetEvent.absoluteY);
    gdk_event_get_coords(sourceEvent, &targetEvent.relativeX, &targetEvent.relativeY);

    // Copy the event button if there is any
    if (targetEvent.type == BUTTON_PRESS_EVENT || targetEvent.type == BUTTON_RELEASE_EVENT) {
        gdk_event_get_button(sourceEvent, &targetEvent.button);
    }
    if (sourceEventType == GDK_TOUCH_BEGIN || sourceEventType == GDK_TOUCH_END || sourceEventType == GDK_TOUCH_CANCEL) {
        // As we only handle single finger events we can set the button statically to 1
        targetEvent.button = 1;
    }
    gdk_event_get_state(sourceEvent, &targetEvent.state);
    if (targetEvent.deviceClass == INPUT_DEVICE_KEYBOARD) {
        gdk_event_get_keyval(sourceEvent, &targetEvent.button);
    }


    // Copy the timestamp
    targetEvent.timestamp = gdk_event_get_time(sourceEvent);

    // Copy the pressure data
    gdk_event_get_axis(sourceEvent, GDK_AXIS_PRESSURE, &targetEvent.pressure);

    // Copy the event sequence if there is any, and report no pressure
    if (sourceEventType == GDK_TOUCH_BEGIN || sourceEventType == GDK_TOUCH_UPDATE || sourceEventType == GDK_TOUCH_END ||
        sourceEventType == GDK_TOUCH_CANCEL) {
        targetEvent.sequence = gdk_event_get_event_sequence(sourceEvent);
        targetEvent.pressure = Point::NO_PRESSURE;
    }

    return targetEvent;
}
