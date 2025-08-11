//
// Created by ulrich on 17.05.19.
//

#include "InputEvents.h"

#include "control/settings/Settings.h"       // for Settings
#include "control/settings/SettingsEnums.h"  // for InputDeviceTypeOption
#include "util/Assert.h"
#include "util/GtkUtil.h"

#include "config-debug.h"  // for DEBUG_INPUT

#ifdef DEBUG_INPUT
template <typename Stream>
Stream& operator<<(Stream& s, const InputEvent& e) {
    static constexpr const char* InputEventClasses[] = {"UNKNOWN",
                                                        "BUTTON_PRESS_EVENT",
                                                        "BUTTON_2_PRESS_EVENT",
                                                        "BUTTON_3_PRESS_EVENT",
                                                        "BUTTON_RELEASE_EVENT",
                                                        "MOTION_EVENT",
                                                        "ENTER_EVENT",
                                                        "LEAVE_EVENT",
                                                        "PROXIMITY_IN_EVENT",
                                                        "PROXIMITY_OUT_EVENT",
                                                        "SCROLL_EVENT",
                                                        "GRAB_BROKEN_EVENT",
                                                        "KEY_PRESS_EVENT",
                                                        "KEY_RELEASE_EVENT"};

    static constexpr const char* InputClasses[] = {"INPUT_DEVICE_MOUSE",    "INPUT_DEVICE_PEN",
                                                   "INPUT_DEVICE_ERASER",   "INPUT_DEVICE_TOUCHSCREEN",
                                                   "INPUT_DEVICE_KEYBOARD", "INPUT_DEVICE_MOUSE_KEYBOARD_COMBO",
                                                   "INPUT_DEVICE_IGNORE"};
    s << "Event:" << "\n | Device: Id:    " << e.deviceId << "\n |         Name:  " << e.deviceName
      << "\n |         Class: " << InputClasses[e.deviceClass] << "\n | Timestamp:     " << e.timestamp
      << "\n | Sequence:      " << e.sequence << "\n | Type:          " << InputEventClasses[e.type];
    if (e.type == BUTTON_PRESS_EVENT || e.type == BUTTON_2_PRESS_EVENT || e.type == BUTTON_3_PRESS_EVENT ||
        e.type == BUTTON_RELEASE_EVENT) {
        s << "\n |                ∟ Button: ";
        switch (e.button) {
            case GDK_BUTTON_PRIMARY:
                s << "primary";
                break;
            case GDK_BUTTON_SECONDARY:
                s << "secondary";
                break;
            case GDK_BUTTON_MIDDLE:
                s << "middle";
                break;
            default:
                s << e.button;
        }
    }
    return s;
}

template <typename Stream>
Stream& operator<<(Stream& s, const GdkEvent* e) {

    static constexpr const char* gdkEventTypes[] = {
            "GDK_DELETE",
            "GDK_MOTION_NOTIFY",
            "GDK_BUTTON_PRESS",
            "GDK_BUTTON_RELEASE",
            "GDK_KEY_PRESS",
            "GDK_KEY_RELEASE",
            "GDK_ENTER_NOTIFY",
            "GDK_LEAVE_NOTIFY",
            "GDK_FOCUS_CHANGE",
            "GDK_PROXIMITY_IN",
            "GDK_PROXIMITY_OUT",
            "GDK_DRAG_ENTER",
            "GDK_DRAG_LEAVE",
            "GDK_DRAG_MOTION",
            "GDK_DROP_START",
            "GDK_SCROLL",
            "GDK_GRAB_BROKEN",
            "GDK_TOUCH_BEGIN",
            "GDK_TOUCH_UPDATE",
            "GDK_TOUCH_END",
            "GDK_TOUCH_CANCEL",
            "GDK_TOUCHPAD_SWIPE",
            "GDK_TOUCHPAD_PINCH",
            "GDK_PAD_BUTTON_PRESS",
            "GDK_PAD_BUTTON_RELEASE",
            "GDK_PAD_RING",
            "GDK_PAD_STRIP",
            "GDK_PAD_GROUP_MODE",
#ifdef GDK_VERSION_4_6
            "GDK_TOUCHPAD_HOLD",
#endif
    };
    static_assert(GDK_EVENT_LAST == G_N_ELEMENTS(gdkEventTypes));

    static constexpr const char* gdkInputSources[] = {
            "GDK_SOURCE_MOUSE",    "GDK_SOURCE_PEN",        "GDK_SOURCE_KEYBOARD",  "GDK_SOURCE_TOUCHSCREEN",
            "GDK_SOURCE_TOUCHPAD", "GDK_SOURCE_TRACKPOINT", "GDK_SOURCE_TABLET_PAD"};

    static constexpr const char* gdkAxisUses[] = {
            "GDK_AXIS_IGNORE  ", "GDK_AXIS_X       ", "GDK_AXIS_Y       ", "GDK_AXIS_DELTA_X ", "GDK_AXIS_DELTA_Y ",
            "GDK_AXIS_PRESSURE", "GDK_AXIS_XTILT   ", "GDK_AXIS_YTILT   ", "GDK_AXIS_WHEEL   ", "GDK_AXIS_DISTANCE",
            "GDK_AXIS_ROTATION", "GDK_AXIS_SLIDER  ", "GDK_AXIS_LAST    "};

    GValue v = G_VALUE_INIT;
    g_object_get_property(G_OBJECT(gdk_event_get_device(const_cast<GdkEvent*>(e))), "n_axes", &v);

    s << " | GDK info:"
      << "\n |     Source: " << gdkInputSources[gdk_device_get_source(gdk_event_get_device(const_cast<GdkEvent*>(e)))]
      << "\n |     Type:   " << gdkEventTypes[gdk_event_get_event_type(const_cast<GdkEvent*>(e))]
      << "\n |     Axes:   " << g_value_get_uint(&v) << " axes supported:";

    double p;
    for (int i = 1; i < GDK_AXIS_LAST; i++) {
        s << "\n |         " << gdkAxisUses[i] << ": ";
        if (gdk_event_get_axis(const_cast<GdkEvent*>(e), (GdkAxisUse)i, &p)) {
            s << "true    value: " << p;
        } else {
            s << "false";
        }
    }
    return s;
}

static void printDebug(const InputEvent& e, GdkEvent* original, guint backlogSize = 0) {
    auto print = [&]() {
        std::cout << e;
        if (backlogSize != 0) {
            std::cout << "\n | Backlog size:  " << backlogSize;
        }
        std::cout << "\n" << original << std::endl;
    };
#ifndef DEBUG_INPUT_PRINT_ALL_MOTION_EVENTS
    static bool motionEventBlock = false;
    if (e.type == MOTION_EVENT) {
        if (!motionEventBlock) {
            motionEventBlock = true;
            print();
        }
    } else {
        motionEventBlock = false;
        print();
    }
#else
    print();
#endif  // DEBUG_INPUT_PRINT_ALL_MOTION_EVENTS
}
#else
static void printDebug(const InputEvent& e, GdkEvent* original, guint backlogSize = 0) {}
#endif  // DEBUG_INPUT


static auto translateEventType(GdkEventType type, int nbPress) -> InputEventType {
    switch (type) {
        case GDK_MOTION_NOTIFY:
        case GDK_TOUCH_UPDATE:
            return MOTION_EVENT;
        case GDK_BUTTON_PRESS:
        case GDK_TOUCH_BEGIN:
            return nbPress == 1 ? BUTTON_PRESS_EVENT : nbPress == 2 ? BUTTON_2_PRESS_EVENT : BUTTON_3_PRESS_EVENT;
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

static xoj::util::Point<double> relativeToAbsolute(const xoj::util::Point<double>& rel, GtkWidget* w) {
    graphene_point_t p = GRAPHENE_POINT_INIT(static_cast<float>(rel.x), static_cast<float>(rel.y));
    graphene_point_t q;
    [[maybe_unused]] bool r = gtk_widget_compute_point(w, gtk_widget_get_ancestor(w, GTK_TYPE_SCROLLED_WINDOW), &p, &q);
    xoj_assert(r);
    return {q.x, q.y};
}

auto InputEvents::translateEvent(GdkEvent* sourceEvent, Settings* settings, GtkWidget* referenceWidget, int nbPress)
        -> std::vector<InputEvent> {
    InputEvent eventMould{};

    eventMould.device = gdk_event_get_device(sourceEvent);

    // Map the event type to our internal ones
    GdkEventType sourceEventType = gdk_event_get_event_type(sourceEvent);
    eventMould.type = translateEventType(sourceEventType, nbPress);

    eventMould.device = gdk_event_get_device(sourceEvent);
    eventMould.deviceClass = translateDeviceType(eventMould.device, settings);

    eventMould.deviceName = gdk_device_get_name(eventMould.device);
    eventMould.deviceId = DeviceId(eventMould.device);

    eventMould.state = gdk_event_get_modifier_state(sourceEvent);
    eventMould.sequence = gdk_event_get_event_sequence(sourceEvent);
    eventMould.timestamp = gdk_event_get_time(sourceEvent);

    bool noPressure = false;
    if (sourceEventType == GDK_TOUCH_BEGIN || sourceEventType == GDK_TOUCH_UPDATE || sourceEventType == GDK_TOUCH_END ||
        sourceEventType == GDK_TOUCH_CANCEL) {
        // As we only handle single finger events we can set the button statically to 1
        eventMould.button = 1;
        noPressure = true;
    } else if (eventMould.type == BUTTON_PRESS_EVENT || eventMould.type == BUTTON_RELEASE_EVENT) {
        eventMould.button = gdk_button_event_get_button(sourceEvent);
        // Nb: BUTTON_PRESS events could have a pressure value (when a stylus tip touches the tablet)
    } else if (eventMould.type == KEY_PRESS_EVENT || eventMould.type == KEY_RELEASE_EVENT) {
        eventMould.button = gdk_key_event_get_keyval(sourceEvent);
        noPressure = true;
        g_warning("Unhandled keyboard event: %d", eventMould.button);
    }

    // Copy both coordinates of the event
    if (xoj::util::Point<double> surfCoord; gdk_event_get_position(sourceEvent, &surfCoord.x, &surfCoord.y)) {
        eventMould.relative = xoj::util::gtk::gdkSurfaceToWidgetCoordinates(surfCoord, referenceWidget);
        eventMould.absolute = relativeToAbsolute(eventMould.relative, referenceWidget);
    } else if (eventMould.type != GRAB_BROKEN_EVENT) {
        g_warning("InputEvents::translateEvent() but GdkEvent has no position");
    }

    // Copy the pressure data
    noPressure = noPressure || !gdk_event_get_axis(sourceEvent, GDK_AXIS_PRESSURE, &eventMould.pressure);
    if (noPressure) {
        eventMould.pressure = Point::NO_PRESSURE;
    }


    if (sourceEventType == GDK_MOTION_NOTIFY) {
        // Fetch history so we have enough events to draw smooth curves
        guint backlogSize = 0;
        GdkTimeCoord* backlog = gdk_event_get_history(sourceEvent, &backlogSize);
        std::vector<InputEvent> res(backlogSize + 1, eventMould);  // One more for the latest event (= eventMould)
        if (backlog) {
            auto it = res.begin();
            for (auto* ev = backlog; ev < backlog + backlogSize; ev++, it++) {
                it->relative = xoj::util::gtk::gdkSurfaceToWidgetCoordinates(
                        {ev->axes[GDK_AXIS_X], ev->axes[GDK_AXIS_Y]}, referenceWidget);
                it->absolute = relativeToAbsolute(it->relative, referenceWidget);
                it->pressure = noPressure ? Point::NO_PRESSURE : ev->axes[GDK_AXIS_PRESSURE];
                it->timestamp = ev->time;
            }
            g_free(backlog);
        }
        printDebug(eventMould, sourceEvent, backlogSize);
        return res;
    } else {
        printDebug(eventMould, sourceEvent);
        return {eventMould};
    }
}
