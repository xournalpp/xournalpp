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

#include <gdk/gdk.h>
#include <glib.h>

#include "control/settings/Settings.h"
#include "model/Point.h"

enum InputEventType {
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

enum InputDeviceClass {
    INPUT_DEVICE_MOUSE,
    INPUT_DEVICE_PEN,
    INPUT_DEVICE_ERASER,
    INPUT_DEVICE_TOUCHSCREEN,
    INPUT_DEVICE_KEYBOARD,
    INPUT_DEVICE_MOUSE_KEYBOARD_COMBO,
    INPUT_DEVICE_IGNORE
};

struct GdkEventGuard {
    static inline GdkEvent* safeRef(GdkEvent* source) { return gdk_event_copy(source); }
    static inline GdkEvent* safeMove(GdkEvent*& source) {
        auto ret = source;
        source = nullptr;
        return ret;
    }
    static inline void safeDelete(GdkEvent* source) {
        if (source != nullptr) {
            gdk_event_free(source);
        }
    }

    GdkEventGuard() = default;
    [[maybe_unused]] explicit GdkEventGuard(GdkEvent* source): event(safeRef(source)) {}
    GdkEventGuard(GdkEventGuard const& other): event(safeRef(other.event)) {}
    GdkEventGuard(GdkEventGuard&& other): event(safeMove(other.event)) {}

    GdkEventGuard& operator=(GdkEvent* source) {
        safeDelete(event);
        event = safeRef(source);
        return *this;
    }
    GdkEventGuard& operator=(GdkEventGuard const& other) {
        safeDelete(event);
        event = safeRef(other.event);
        return *this;
    }
    GdkEventGuard& operator=(GdkEventGuard&& other) {
        safeDelete(event);
        event = safeMove(other.event);
        return *this;
    }

    operator GdkEvent*() const { return event; }

    ~GdkEventGuard() { safeDelete(event); }

    GdkEvent* event{};
};


struct InputEvent final {
    /*explicit(false)*/ explicit operator bool() const { return sourceEvent.event; }

    GdkEventGuard sourceEvent;

    InputEventType type{UNKNOWN};
    InputDeviceClass deviceClass{INPUT_DEVICE_IGNORE};
    gchar* deviceName{};

    gdouble absoluteX{0};
    gdouble absoluteY{0};
    gdouble relativeX{0};
    gdouble relativeY{0};

    guint button{0};
    GdkModifierType state{};
    gdouble pressure{Point::NO_PRESSURE};

    GdkEventSequence* sequence{};
    guint32 timestamp{0};
};

class InputEvents {

    static InputEventType translateEventType(GdkEventType type);

public:
    static InputDeviceClass translateDeviceType(GdkDevice* device, Settings* settings);
    static InputDeviceClass translateDeviceType(const string& name, GdkInputSource source, Settings* settings);

    static InputEvent translateEvent(GdkEvent* sourceEvent, Settings* settings);
};
