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

#include <memory>  // for shared_ptr
#include <string>  // for string

#include <gdk/gdk.h>  // for GdkEvent, gdk_event_free, gdk_event_copy
#include <glib.h>     // for gdouble, gchar, guint, guint32
#include <gtk/gtk.h>

#include "model/Point.h"  // for Point::NO_PRESSURE
#include "util/Point.h"
#include "util/raii/CLibrariesSPtr.h"
#include "util/raii/IdentityFunction.h"

#include "DeviceId.h"

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

class GdkEventHandler {
public:
    constexpr static auto ref = gdk_event_ref;
    constexpr static auto unref = gdk_event_unref;
    constexpr static auto adopt = xoj::util::specialization::identity<GdkEvent>;
};

struct InputEvent final {
    /*explicit(false)*/ explicit operator bool() const { return sourceEvent; }

    xoj::util::CLibrariesSPtr<GdkEvent, GdkEventHandler> sourceEvent;  ///< Original GdkEvent. Avoid using if possible.

    InputEventType type{UNKNOWN};
    InputDeviceClass deviceClass{INPUT_DEVICE_IGNORE};
    const gchar* deviceName{};

    xoj::util::Point<double> absolute;  ///< In GdkSurface coordinates
    xoj::util::Point<double> relative;  ///< In XournalWidget coordinates

    guint button{0};
    GdkModifierType state{};
    gdouble pressure{Point::NO_PRESSURE};

    GdkEventSequence* sequence{};
    guint32 timestamp{0};

    DeviceId deviceId;
};

struct KeyEvent final {
    guint keyval{0};
    GdkModifierType state{};  ///< Consumed modifiers have been masked out

    xoj::util::CLibrariesSPtr<GdkEvent, GdkEventHandler> sourceEvent;  ///< Original GdkEvent. Avoid using if possible.
};

class InputEvents {

    static InputEventType translateEventType(GdkEventType type);

public:
    static InputDeviceClass translateDeviceType(GdkDevice* device, Settings* settings);
    static InputDeviceClass translateDeviceType(const std::string& name, GdkInputSource source, Settings* settings);

    static InputEvent translateEvent(GdkEvent* sourceEvent, Settings* settings, GtkWidget* referenceWidget);
};
