//
// Created by ulrich on 06.04.19.
//

#include "InputContext.h"

#include <cstddef>  // for NULL
#include <vector>   // for vector

#include <glib-object.h>  // for g_signal_hand...

#include "control/Control.h"                            // for Control
#include "control/DeviceListHelper.h"                   // for InputDevice
#include "control/settings/Settings.h"                  // for Settings
#include "gui/XournalView.h"                            // for XournalView
#include "gui/inputdevices/GeometryToolInputHandler.h"  // for GeometryToolInputHandler
#include "gui/inputdevices/HandRecognition.h"           // for HandRecognition
#include "gui/inputdevices/KeyboardInputHandler.h"      // for KeyboardInput...
#include "gui/inputdevices/MouseInputHandler.h"         // for MouseInputHan...
#include "gui/inputdevices/StylusInputHandler.h"        // for StylusInputHa...
#include "gui/inputdevices/TouchDrawingInputHandler.h"  // for TouchDrawingI...
#include "gui/inputdevices/TouchInputHandler.h"         // for TouchInputHan...
#include "util/Assert.h"                                // for xoj_assert
#include "util/glib_casts.h"                            // for wrap_for_g_callback

#include "InputEvents.h"   // for InputEvent
#include "config-debug.h"  // for DEBUG_INPUT

class ScrollHandling;
class ToolHandler;

InputContext::InputContext(XournalView* view, ScrollHandling* scrollHandling):
        view(view),
        scrollHandling(scrollHandling),
        stylusHandler(std::make_unique<StylusInputHandler>(this)),
        mouseHandler(std::make_unique<MouseInputHandler>(this)),
        touchDrawingHandler(std::make_unique<TouchDrawingInputHandler>(this)),
        keyboardHandler(std::make_unique<KeyboardInputHandler>(this)),
        touchHandler(std::make_unique<TouchInputHandler>(this)) {
    for (const InputDevice& savedDevices: this->view->getControl()->getSettings()->getKnownInputDevices()) {
        this->knownDevices.insert(savedDevices.getName());
    }
}

template <bool (KeyboardInputHandler::*handler)(KeyEvent) const>
static gboolean keyboardCallback(GtkEventControllerKey* self, guint keyval, guint, GdkModifierType state, gpointer d) {
    auto* gdkEvent = gtk_event_controller_get_current_event(GTK_EVENT_CONTROLLER(self));
    KeyEvent e;
    e.keyval = keyval;
    e.state = static_cast<GdkModifierType>(state & gtk_accelerator_get_default_mod_mask() &
                                           ~gdk_key_event_get_consumed_modifiers(gdkEvent));
    e.sourceEvent.reset(gdkEvent, xoj::util::ref);
    return (static_cast<KeyboardInputHandler*>(d)->*handler)(std::move(e));
}

InputContext::~InputContext() {
    // Destructor is called in xournal_widget_dispose, so it can still accept events
    for (auto* ctrl: eventControllers) {
        gtk_widget_remove_controller(this->widget, ctrl);
    }
}

void InputContext::connect(GtkWidget* pWidget) {
    xoj_assert(!this->widget && pWidget);
    this->widget = pWidget;

    auto* legCtrl = gtk_event_controller_legacy_new();
    g_signal_connect(legCtrl, "event", xoj::util::wrap_for_g_callback_v<eventCallback>, this);
    gtk_event_controller_set_propagation_phase(legCtrl, GTK_PHASE_TARGET);
    gtk_widget_add_controller(widget, legCtrl);
    eventControllers.push_back(legCtrl);

    // The last added GtkEventController gets called first: the GtkeventControllerKey gets a chance to grab the events,
    // avoiding unnecessary calls of the GtkEventControllerLegacy
    auto* keyCtrl = gtk_event_controller_key_new();
    gtk_widget_add_controller(widget, keyCtrl);
    gtk_event_controller_set_propagation_phase(keyCtrl, GTK_PHASE_TARGET);
    eventControllers.push_back(keyCtrl);

    g_signal_connect(keyCtrl, "key-pressed", G_CALLBACK(keyboardCallback<&KeyboardInputHandler::keyPressed>),
                     keyboardHandler.get());
    g_signal_connect(keyCtrl, "key-released", G_CALLBACK(keyboardCallback<&KeyboardInputHandler::keyReleased>),
                     keyboardHandler.get());

    auto* clickCtrl = gtk_gesture_click_new();
    gtk_gesture_single_set_exclusive(GTK_GESTURE_SINGLE(clickCtrl), true);  // Only handle pointer or pointer emulated
    gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(clickCtrl), 0);        // Take any button into account
    g_signal_connect(clickCtrl, "pressed",
                     G_CALLBACK(+[](GtkGestureClick* ctrl, gint n_press, gdouble, gdouble, gpointer d) {
                         auto* self = static_cast<InputContext*>(d);
                         GdkEvent* event = gtk_event_controller_get_current_event(GTK_EVENT_CONTROLLER(ctrl));
                         if (gdk_event_get_device(event) == NULL) {
                             return;
                         }
                         auto e = InputEvents::translateEvent(event, self->getSettings(), self->widget);

                         // Always issue a BUTTON_PRESS_EVENT, see XojPageView::onButton{Double,Triple}PressEvent()
                         self->handle(e);

                         // Only handle double/triple clicks of the primary button
                         if (gtk_gesture_single_get_current_button(GTK_GESTURE_SINGLE(ctrl)) == GDK_BUTTON_PRIMARY) {
                             if (n_press == 2) {
                                 e.type = BUTTON_2_PRESS_EVENT;
                                 self->handle(std::move(e));
                             } else if (n_press == 3) {
                                 e.type = BUTTON_3_PRESS_EVENT;
                                 self->handle(std::move(e));
                             }
                         }
                     }),
                     this);
    gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER(clickCtrl), GTK_PHASE_TARGET);
    gtk_widget_add_controller(widget, GTK_EVENT_CONTROLLER(clickCtrl));
    eventControllers.push_back(GTK_EVENT_CONTROLLER(clickCtrl));
}


static constexpr bool HANDLE_EVENT_TYPE[] = {
        false,  // GDK_DELETE
        true,   // GDK_MOTION_NOTIFY
        false,  // GDK_BUTTON_PRESS    -- Handled by the GtkGestureClick
        true,   // GDK_BUTTON_RELEASE
        false,  // GDK_KEY_PRESS       -- Handled by the GtkEventControllerKey
        false,  // GDK_KEY_RELEASE     -- Handled by the GtkEventControllerKey
        true,   // GDK_ENTER_NOTIFY
        true,   // GDK_LEAVE_NOTIFY
        false,  // GDK_FOCUS_CHANGE
        true,   // GDK_PROXIMITY_IN
        true,   // GDK_PROXIMITY_OUT
        false,  // GDK_DRAG_ENTER
        false,  // GDK_DRAG_LEAVE
        false,  // GDK_DRAG_MOTION
        false,  // GDK_DROP_START
        false,  // GDK_SCROLL          -- Handled by GTK's default handler or in ZoomControl's GtkEventControllerScroll
        true,   // GDK_GRAB_BROKEN
        true,   // GDK_TOUCH_BEGIN
        true,   // GDK_TOUCH_UPDATE
        true,   // GDK_TOUCH_END
        true,   // GDK_TOUCH_CANCEL
        false,  // GDK_TOUCHPAD_SWIPE
        false,  // GDK_TOUCHPAD_PINCH
        false,  // GDK_PAD_BUTTON_PRESS
        false,  // GDK_PAD_BUTTON_RELEASE
        false,  // GDK_PAD_RING
        false,  // GDK_PAD_STRIP
        false,  // GDK_PAD_GROUP_MODE
#ifdef GDK_VERSION_4_6
        false,  // GDK_TOUCHPAD_HOLD
#endif
};
static_assert(GDK_EVENT_LAST == G_N_ELEMENTS(HANDLE_EVENT_TYPE));
static bool shouldHandleEvent(GdkEvent* e) { return e && HANDLE_EVENT_TYPE[gdk_event_get_event_type(e)]; }

auto InputContext::eventCallback(GtkEventControllerLegacy*, GdkEvent* event, InputContext* self) -> bool {
    if (!shouldHandleEvent(event)) {
        return false;
    }

    if (gdk_event_get_device(event) == NULL) {
        return false;
    }

    return self->handle(InputEvents::translateEvent(event, self->getSettings(), self->widget));
}

auto InputContext::handle(InputEvent event) -> bool {
    printDebug(event);

    // Add the device to the list of known devices if it is currently unknown
    GdkInputSource inputSource = gdk_device_get_source(gdk_event_get_device(event.sourceEvent.get()));
    if (inputSource != GDK_SOURCE_KEYBOARD &&
        this->knownDevices.find(std::string(event.deviceName)) == this->knownDevices.end()) {

        this->knownDevices.insert(std::string(event.deviceName));
        this->getSettings()->transactionStart();
        auto deviceClassOption =
                this->getSettings()->getDeviceClassForDevice(std::string(event.deviceName), inputSource);
        this->getSettings()->setDeviceClassForDevice(gdk_event_get_device(event.sourceEvent.get()), deviceClassOption);
        this->getSettings()->transactionEnd();
    }

    // Deactivate touchscreen when a pen event occurs
    this->getView()->getHandRecognition()->event(event.deviceClass);

    // Get the state of all modifiers
    this->modifierState = event.state;

    // separate events to appropriate handlers
    // handle geometry tool
    if (geometryToolInputHandler && geometryToolInputHandler->handle(event)) {
        return true;
    }

    // handle tablet stylus
    if (event.deviceClass == INPUT_DEVICE_PEN || event.deviceClass == INPUT_DEVICE_ERASER) {
        return this->stylusHandler->handle(event);
    }

    // handle mouse devices
    if (event.deviceClass == INPUT_DEVICE_MOUSE || event.deviceClass == INPUT_DEVICE_MOUSE_KEYBOARD_COMBO) {
        return this->mouseHandler->handle(event);
    }

    // handle touchscreens
    if (event.deviceClass == INPUT_DEVICE_TOUCHSCREEN) {
        bool touchDrawingEnabled = this->getSettings()->getTouchDrawingEnabled();

        // trigger touch drawing depending on the setting
        if (touchDrawingEnabled) {
            return this->touchDrawingHandler->handle(event) || this->touchHandler->handle(event);
        }

        return this->touchHandler->handle(event);
    }

    if (event.deviceClass == INPUT_DEVICE_IGNORE) {
        return true;
    }

#ifdef DEBUG_INPUT
    g_message("We received an event we do not have a handler for");
#endif
    return false;
}

auto InputContext::getXournal() -> GtkXournal* { return GTK_XOURNAL(widget); }

auto InputContext::getView() -> XournalView* { return view; }

auto InputContext::getSettings() -> Settings* { return view->getControl()->getSettings(); }

auto InputContext::getToolHandler() -> ToolHandler* { return view->getControl()->getToolHandler(); }

auto InputContext::getScrollHandling() -> ScrollHandling* { return this->scrollHandling; }

void InputContext::setGeometryToolInputHandler(std::unique_ptr<GeometryToolInputHandler> handler) {
    this->geometryToolInputHandler = std::move(handler);
}

auto InputContext::getGeometryToolInputHandler() const -> GeometryToolInputHandler* {
    return geometryToolInputHandler.get();
}

void InputContext::resetGeometryToolInputHandler() { this->geometryToolInputHandler.reset(); }

auto InputContext::getModifierState() -> GdkModifierType { return this->modifierState; }

/**
 * Focus the widget
 */
void InputContext::focusWidget() {
    if (!gtk_widget_has_focus(widget)) {
        gtk_widget_grab_focus(widget);
    }
}

void InputContext::blockDevice(InputContext::DeviceType deviceType) {
    if (geometryToolInputHandler) {
        geometryToolInputHandler->blockDevice(deviceType);
    }
    switch (deviceType) {
        case MOUSE:
            this->mouseHandler->block(true);
            break;
        case STYLUS:
            this->stylusHandler->block(true);
            break;
        case TOUCHSCREEN:
            this->touchDrawingHandler->block(true);
            this->touchHandler->block(true);
            break;
    }
}

void InputContext::unblockDevice(InputContext::DeviceType deviceType) {
    if (geometryToolInputHandler) {
        geometryToolInputHandler->unblockDevice(deviceType);
    }
    switch (deviceType) {
        case MOUSE:
            this->mouseHandler->block(false);
            break;
        case STYLUS:
            this->stylusHandler->block(false);
            break;
        case TOUCHSCREEN:
            this->touchDrawingHandler->block(false);
            this->touchHandler->block(false);
            break;
    }
}

auto InputContext::isBlocked(InputContext::DeviceType deviceType) -> bool {
    switch (deviceType) {
        case MOUSE:
            return this->mouseHandler->isBlocked();
        case STYLUS:
            return this->stylusHandler->isBlocked();
        case TOUCHSCREEN:
            return this->touchDrawingHandler->isBlocked();
    }
    return false;
}

#ifdef DEBUG_INPUT
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

static constexpr const char* InputEventClasses[] = {
        "UNKNOWN",      "BUTTON_PRESS_EVENT", "BUTTON_2_PRESS_EVENT", "BUTTON_3_PRESS_EVENT", "BUTTON_RELEASE_EVENT",
        "MOTION_EVENT", "ENTER_EVENT",        "LEAVE_EVENT",          "PROXIMITY_IN_EVENT",   "PROXIMITY_OUT_EVENT",
        "SCROLL_EVENT", "GRAB_BROKEN_EVENT",  "KEY_PRESS_EVENT",      "KEY_RELEASE_EVENT"};

static constexpr const char* InputClasses[] = {"INPUT_DEVICE_MOUSE",    "INPUT_DEVICE_PEN",
                                               "INPUT_DEVICE_ERASER",   "INPUT_DEVICE_TOUCHSCREEN",
                                               "INPUT_DEVICE_KEYBOARD", "INPUT_DEVICE_MOUSE_KEYBOARD_COMBO",
                                               "INPUT_DEVICE_IGNORE"};
#endif

void InputContext::printDebug(const InputEvent& event) {
#ifdef DEBUG_INPUT
    auto type = gdk_event_get_event_type(event.sourceEvent.get());
    std::string message;
    message.reserve(200);
    message += "Event\n";
    ((message += "  | Event type   :\t") += gdkEventTypes[type]) += "\n";
    ((message += "  | Event class  :\t") += InputEventClasses[event.type]) += "\n";

    if (type == GDK_BUTTON_PRESS || type == GDK_BUTTON_RELEASE) {
        ((message += "  | Button       :\t") += std::to_string(gdk_button_event_get_button(event.sourceEvent.get()))) +=
                "\n";
    }

    ((message += "  | Source device:\t") +=
     gdkInputSources[gdk_device_get_source(gdk_event_get_device(event.sourceEvent.get()))]) += "\n";
    ((message += "  ∟ Device Class :\t") += InputClasses[event.deviceClass]) += "\n";

#ifndef DEBUG_INPUT_PRINT_ALL_MOTION_EVENTS
    static bool motionEventBlock = false;
    if (gdk_event_get_event_type(event.sourceEvent.get()) == GDK_MOTION_NOTIFY) {
        if (!motionEventBlock) {
            motionEventBlock = true;
            g_message("%s", message.c_str());
        }
    } else {
        motionEventBlock = false;
        g_message("%s", message.c_str());
    }
#else
    g_message("%s", message.c_str());
#endif  // DEBUG_INPUT_PRINT_ALL_MOTION_EVENTS
#endif  // DEBUG_INPUT
}
