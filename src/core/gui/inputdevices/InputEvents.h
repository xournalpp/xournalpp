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

#include <memory>

#include <gdk/gdk.h>
#include <glib.h>

#include "model/Point.h"

class Settings;

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

    GdkEventGuard() = default;

    [[maybe_unused]] explicit GdkEventGuard(GdkEvent* source): event(safeRef(source), &gdk_event_free) {}

    GdkEventGuard& operator=(GdkEvent* source) {
        event = {safeRef(source), &gdk_event_free};
        return *this;
    }

    operator GdkEvent*() const { return event.get(); }

    // it's more performant to manage the GdkEvent over C++ than over gdk
    // Since the gdk_copy is extreme expansive
    std::shared_ptr<GdkEvent> event{};
};

struct InputEvent final {
    /*explicit(false)*/ explicit operator bool() const { return !!sourceEvent.event; }

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
    static InputDeviceClass translateDeviceType(const std::string& name, GdkInputSource source, Settings* settings);

    static InputEvent translateEvent(GdkEvent* sourceEvent, Settings* settings);
};
