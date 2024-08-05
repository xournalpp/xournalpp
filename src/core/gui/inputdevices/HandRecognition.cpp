#include "HandRecognition.h"

#include <cstring>  // for strcmp
#include <string>   // for all...

#include "control/settings/Settings.h"                            // for SEl...
#include "gtk/gtk.h"                                              // for Gtk...
#include "gui/inputdevices/InputEvents.h"                         // for INP...
#include "gui/inputdevices/touchdisable/TouchDisableCustom.h"     // for Tou...
#include "gui/inputdevices/touchdisable/TouchDisableInterface.h"  // for Tou...
#include "gui/inputdevices/touchdisable/TouchDisableX11.h"        // for Tou...
#include "util/glib_casts.h"                                      // for wrap_v
#include "util/safe_casts.h"                                      // for as_unsigned

#include "InputContext.h"  // for Inp...

using std::string;

HandRecognition::HandRecognition(GtkWidget* widget, InputContext* inputContext, Settings* settings):
        inputContext(inputContext), settings(settings) {
#ifdef X11_ENABLED
    const char* sessionType = g_getenv("XDG_SESSION_TYPE");
    if (sessionType != nullptr && strcmp(sessionType, "x11") == 0) {
        x11Session = true;
    }
#endif

    reload();
}

HandRecognition::~HandRecognition() {
    // Enable touchscreen on quit application
    if (!touchState && enabled && touchImpl) {
        touchImpl->enableTouch();
    }

    delete touchImpl;
    touchImpl = nullptr;
}

/**
 * Reload settings
 */
void HandRecognition::reload() {
    SElement& touch = settings->getCustomElement("touch");

    enabled = false;
    touch.getBool("disableTouch", enabled);

    if (!enabled) {
        delete touchImpl;
        touchImpl = nullptr;
        return;
    }

    disableTimeout = 1000;
    touch.getInt("timeout", disableTimeout);
    if (disableTimeout < 500) {
        disableTimeout = 500;
    }

    delete touchImpl;
    touchImpl = nullptr;

    string disableMethod;
    touch.getString("method", disableMethod);
    if (disableMethod == "X11") {
        if (!x11Session) {
            g_warning("X11 Touch workaround is selected, but no X11 Session running!");
            enabled = false;
            return;
        }
#ifdef X11_ENABLED
        g_warning("Fix X11 touch-disabled");
        // touchImpl = new TouchDisableX11();
#endif
    } else if (disableMethod == "custom") {
        string enableCommand;
        touch.getString("cmdEnable", enableCommand);
        string disableCommand;
        touch.getString("cmdDisable", disableCommand);

        touchImpl = new TouchDisableCustom(enableCommand, disableCommand);
    } else  // Auto detect
    {
#ifdef X11_ENABLED
        // if (x11Session) {
        //     touchImpl = new TouchDisableX11();
        // }
#endif
    }

    if (touchImpl) {
        touchImpl->init();
    }
}

/**
 * Called after the timeout
 *
 * @return true to call again
 */
auto HandRecognition::enableTimeout(HandRecognition* self) -> bool {
    gint64 now = g_get_monotonic_time() / 1000;
    gint64 lastPenActionTime = now - self->lastPenAction;
    if (lastPenActionTime < 20) {
        // Pen action within the last 20ms, so simple restart timeout
        return true;
    }

    if (lastPenActionTime > self->disableTimeout) {
        // Timeout elapsed, enable touch again
        self->enableTouch();
        self->touchState = true;

        // Do not call again
        return false;
    }

    auto nextTime = static_cast<guint>(now - self->lastPenAction + self->disableTimeout);

    g_timeout_add(nextTime, xoj::util::wrap_v<enableTimeout>, self);

    // Do not call again, a new time is scheduled
    return false;
}

/**
 * There was a pen event, restart the timer
 */
void HandRecognition::penEvent() {
    lastPenAction = g_get_monotonic_time() / 1000;

    if (touchState) {
        touchState = false;
        if (enabled) {
            disableTouch();
        }
        g_timeout_add(as_unsigned(disableTimeout), xoj::util::wrap_v<enableTimeout>, this);
    }
}

/**
 * Enable touchscreen
 */
void HandRecognition::enableTouch() {
    if (inputContext) {
        inputContext->unblockDevice(InputContext::DeviceType::TOUCHSCREEN);
    }
    if (touchImpl && enabled) {
        touchImpl->enableTouch();
    }
}

/**
 * Disable touchscreen
 */
void HandRecognition::disableTouch() {
    if (inputContext) {
        inputContext->blockDevice(InputContext::DeviceType::TOUCHSCREEN);
    }
    if (touchImpl) {
        touchImpl->disableTouch();
    }
}

/**
 * An event from a device occurred
 */
void HandRecognition::event(InputDeviceClass device) {
    if (!enabled) {
        return;
    }
    if (device == INPUT_DEVICE_PEN || device == INPUT_DEVICE_ERASER) {
        penEvent();
    }
}

void HandRecognition::unblock() {
    this->enableTouch();
    this->touchState = true;
}
