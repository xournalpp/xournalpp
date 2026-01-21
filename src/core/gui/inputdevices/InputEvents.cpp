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
#include "PrintEvent.h"

static void printDebug(const InputEvent& e, GdkEvent* original, guint backlogSize = 0) {
    auto print = [&]() {
        xoj::input::printEvent(std::cout, e, 0);
        if (backlogSize != 0) {
            std::cout << "   Backlog size:  " << backlogSize << "\n";
        }
        xoj::input::printGdkEvent(std::cout, original, 0);
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
        default:
            // Events we do not care about or handle otherwise (e.g. key events)
            return UNKNOWN;
    }
}

auto InputEvents::translateDeviceType(const std::string& name, GdkInputSource source, Settings* settings)
        -> InputDeviceClass {
    InputDeviceTypeOption deviceType = settings->getDeviceClassForDevice(name, source);
    switch (deviceType) {
        case InputDeviceTypeOption::Disabled:
            return INPUT_DEVICE_IGNORE;
        case InputDeviceTypeOption::Mouse:
        case InputDeviceTypeOption::MouseKeyboardCombo:
            return INPUT_DEVICE_MOUSE;
        case InputDeviceTypeOption::Pen:
            return INPUT_DEVICE_PEN;
        case InputDeviceTypeOption::Eraser:
            return INPUT_DEVICE_ERASER;
        case InputDeviceTypeOption::Touchscreen:
            return INPUT_DEVICE_TOUCHSCREEN;
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
