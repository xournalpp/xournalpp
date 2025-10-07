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
#include "gui/dialog/DeviceTestingArea.h"               // for DeviceTestingArea
#include "gui/inputdevices/GeometryToolInputHandler.h"  // for GeometryToolInputHandler
#include "gui/inputdevices/HandRecognition.h"           // for HandRecognition
#include "gui/inputdevices/KeyboardInputHandler.h"      // for KeyboardInput...
#include "gui/inputdevices/MouseInputHandler.h"         // for MouseInputHan...
#include "gui/inputdevices/StylusInputHandler.h"        // for StylusInputHa...
#include "gui/inputdevices/TouchDrawingInputHandler.h"  // for TouchDrawingI...
#include "gui/inputdevices/TouchInputHandler.h"         // for TouchInputHan...
#include "util/Assert.h"                                // for xoj_assert
#include "util/gdk4_helper.h"
#include "util/glib_casts.h"  // for wrap_for_g_callback
#include "util/gtk4_helper.h"

#include "InputEvents.h"   // for InputEvent
#include "config-debug.h"  // for DEBUG_INPUT

#ifdef DEBUG_INPUT
#include "PrintEvent.h"
#endif

class ScrollHandling;
class ToolHandler;

InputContext::InputContext(XournalView* view, ScrollHandling* scrollHandling):
        view(view),
        scrollHandling(scrollHandling),
        settings(view->getControl()->getSettings()),
        stylusHandler(std::make_unique<StylusInputHandler>(this)),
        mouseHandler(std::make_unique<MouseInputHandler>(this)),
        touchDrawingHandler(std::make_unique<TouchDrawingInputHandler>(this)),
        keyboardHandler(std::make_unique<KeyboardInputHandler>(this)),
        touchHandler(std::make_unique<TouchInputHandler>(this)),
        handRecognition(std::make_unique<HandRecognition>(this)) {
    for (const InputDevice& savedDevices: this->view->getControl()->getSettings()->getKnownInputDevices()) {
        this->knownDevices.insert(savedDevices.getName());
    }
}

InputContext::InputContext(Settings* settings, DeviceTestingArea& testing):
        view(nullptr),
        scrollHandling(nullptr),
        settings(settings),
        stylusHandler(std::make_unique<TestingHandler<StylusInputHandler>>(this, DeviceTestingArea::HandlerType::STYLUS,
                                                                           testing)),
        mouseHandler(std::make_unique<TestingHandler<MouseInputHandler>>(this, DeviceTestingArea::HandlerType::MOUSE,
                                                                         testing)),
        touchHandler(std::make_unique<TestingHandler<TouchInputHandler>>(this, DeviceTestingArea::HandlerType::TOUCH,
                                                                         testing)),
        handRecognition(std::make_unique<HandRecognition>(this)) {
    for (const InputDevice& savedDevices: settings->getKnownInputDevices()) {
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
    e.sourceEvent = gdkEvent;
    return (static_cast<KeyboardInputHandler*>(d)->*handler)(e);
}

InputContext::~InputContext() {
    // Destructor is called in xournal_widget_dispose, so it can still accept events
    g_signal_handler_disconnect(this->widget, signal_id);
}

void InputContext::connect(GtkWidget* pWidget, bool connectKeyboardHandler,
                           std::optional<std::function<void(GdkEvent*)>> logfunction) {
    xoj_assert(!this->widget && pWidget);
    this->widget = pWidget;
    gtk_widget_set_support_multidevice(widget, true);

    if (connectKeyboardHandler) {
#if GTK_MAJOR_VERSION == 3
        auto* keyCtrl = gtk_event_controller_key_new(widget);
#else
        auto* keyCtrl = gtk_event_controller_key_new();
        gtk_widget_add_controller(keyCtrl);
#endif

        g_signal_connect(keyCtrl, "key-pressed", G_CALLBACK(keyboardCallback<&KeyboardInputHandler::keyPressed>),
                         keyboardHandler.get());
        g_signal_connect(keyCtrl, "key-released", G_CALLBACK(keyboardCallback<&KeyboardInputHandler::keyReleased>),
                         keyboardHandler.get());
    }

    int mask =
            // Allow scrolling
            GDK_SCROLL_MASK | GDK_SMOOTH_SCROLL_MASK |

            // Touch / Pen / Mouse
            GDK_TOUCH_MASK | GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
            GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK | GDK_PROXIMITY_IN_MASK | GDK_PROXIMITY_OUT_MASK;

    gtk_widget_add_events(pWidget, mask);


    if (!logfunction) {
        signal_id =
                g_signal_connect(pWidget, "event", G_CALLBACK(+[](GtkWidget* widget, GdkEvent* event, gpointer self) {
                                     return static_cast<InputContext*>(self)->handle(event);
                                 }),
                                 this);
    } else {
        struct D {
            InputContext* self;
            std::function<void(GdkEvent*)> logfunction;
        };
        signal_id = g_signal_connect_data(
                pWidget, "event", G_CALLBACK(+[](GtkWidget* widget, GdkEvent* event, gpointer d) {
                    auto* data = static_cast<D*>(d);
                    data->logfunction(event);
                    return data->self->handle(event);
                }),
                new D{this, logfunction.value()}, xoj::util::closure_notify_cb<D>, GConnectFlags(0));
    }
}

auto InputContext::handle(GdkEvent* sourceEvent) -> bool {
    printDebug(sourceEvent);

    GdkDevice* sourceDevice = gdk_event_get_source_device(sourceEvent);
    if (sourceDevice == NULL) {
        return false;
    }

    GdkInputSource inputSource = gdk_device_get_source(sourceDevice);
    if (inputSource == GDK_SOURCE_KEYBOARD) {
        // Keyboard events are handled via the GtkEventControllerKey instance
        return false;
    }

    InputEvent event = InputEvents::translateEvent(sourceEvent, this->getSettings());

    // Add the device to the list of known devices if it is currently unknown
    if (gdk_device_get_device_type(sourceDevice) != GDK_DEVICE_TYPE_MASTER &&
        this->knownDevices.find(std::string(event.deviceName)) == this->knownDevices.end()) {

        this->knownDevices.insert(std::string(event.deviceName));
        this->getSettings()->transactionStart();
        auto deviceClassOption =
                this->getSettings()->getDeviceClassForDevice(std::string(event.deviceName), inputSource);
        this->getSettings()->setDeviceClassForDevice(sourceDevice, deviceClassOption);
        this->getSettings()->transactionEnd();
    }

    // We do not handle scroll events manually but let GTK do it for us
    if (event.type == SCROLL_EVENT) {
        // Hand over to standard GTK Scroll / Zoom handling
        return false;
    }

    // Deactivate touchscreen when a pen event occurs
    this->handRecognition->event(event.deviceClass);

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
    if (event.deviceClass == INPUT_DEVICE_MOUSE) {
        return this->mouseHandler->handle(event);
    }

    // handle touchscreens
    if (event.deviceClass == INPUT_DEVICE_TOUCHSCREEN) {
        bool touchDrawingEnabled = this->getSettings()->getTouchDrawingEnabled();

        // trigger touch drawing depending on the setting
        if (touchDrawingEnabled && this->touchDrawingHandler) {
            return this->touchDrawingHandler->handle(event) || this->touchHandler->handle(event);
        }

        return this->touchHandler->handle(event);
    }

    if (event.deviceClass == INPUT_DEVICE_IGNORE) {
        return true;
    }
    return false;
}

auto InputContext::getXournal() const -> GtkXournal* { return GTK_XOURNAL(widget); }

auto InputContext::getView() const -> XournalView* { return view; }

auto InputContext::getSettings() const -> Settings* { return settings; }

auto InputContext::getToolHandler() const -> ToolHandler* { return view->getControl()->getToolHandler(); }

auto InputContext::getScrollHandling() const -> ScrollHandling* { return this->scrollHandling; }

auto InputContext::getHandRecognition() const -> HandRecognition* { return this->handRecognition.get(); }

void InputContext::setGeometryToolInputHandler(std::unique_ptr<GeometryToolInputHandler> handler) {
    this->geometryToolInputHandler = std::move(handler);
}

auto InputContext::getGeometryToolInputHandler() const -> GeometryToolInputHandler* {
    return geometryToolInputHandler.get();
}

void InputContext::resetGeometryToolInputHandler() { this->geometryToolInputHandler.reset(); }

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

void InputContext::printDebug(GdkEvent* event) {
#ifdef DEBUG_INPUT_GDK_PRINT_EVENTS
    gdk_set_show_events(true);
#else
#ifdef DEBUG_INPUT
    auto print = [&]() {
        GdkDevice* device = gdk_event_get_source_device(event);
        if (device) {
            static constexpr const char* inputClassToString[] = {"INPUT_DEVICE_MOUSE", "INPUT_DEVICE_PEN",
                                                                 "INPUT_DEVICE_ERASER", "INPUT_DEVICE_TOUCHSCREEN",
                                                                 "INPUT_DEVICE_IGNORE"};
            InputDeviceClass deviceClass = InputEvents::translateDeviceType(device, this->getSettings());
            std::cout << "Device Class:\t" << inputClassToString[deviceClass] << "\treceived:\n";
        } else {
            std::cout << "Device Class:\t: none (source device is NULL)\n";
        }
        xoj::input::printGdkEvent(std::cout, event, 0);
    };

#ifndef DEBUG_INPUT_PRINT_ALL_MOTION_EVENTS
    static bool motionEventBlock = false;
    if (gdk_event_get_event_type(event) == GDK_MOTION_NOTIFY) {
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
#endif  // DEBUG_INPUT
#endif  // DEBUG_INPUT_PRINT_EVENTS
}
