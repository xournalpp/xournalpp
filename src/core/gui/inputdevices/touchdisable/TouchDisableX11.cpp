#include "TouchDisableX11.h"

#ifdef X11_ENABLED

#include <X11/Xatom.h>          // for XA_INTEGER
#include <X11/extensions/XI.h>  // for XI_TOUCHSCREEN
#include <gdk/gdk.h>            // for gdk_display_get_default
#include <gdk/gdkx.h>           // for gdk_x11_display_get_xdisplay
#include <glib.h>               // for g_message, g_warning, g_error

TouchDisableX11::TouchDisableX11() = default;

TouchDisableX11::~TouchDisableX11() {
    if (touchdev) {
        XCloseDevice(display, touchdev);
        touchdev = nullptr;
    }

    display = nullptr;
}

void TouchDisableX11::init() {
    // Get display from GTK
    display = gdk_x11_display_get_xdisplay(gdk_display_get_default());
    if (display == nullptr) {
        g_error("Could not open X11 display");
        return;
    }

    touchAtom = XInternAtom(display, "TOUCHSCREEN", true);
    if (touchAtom == None) {
        touchAtom = XInternAtom(display, XI_TOUCHSCREEN, false);
    }


    int inputDeviceCount = 0;
    XID touchId = 0;
    XDeviceInfo* devices = XListInputDevices(display, &inputDeviceCount);
    for (int i = 0; i < inputDeviceCount; i++) {
        if (touchId == 0 && devices[i].type == touchAtom) {
            touchId = devices[i].id;
        }

        if (devices[i].id == touchId) {
            touch = devices + i;
        }
    }

    if (touch == nullptr) {
        g_warning("Could not find touchscreen device for disabling");
        return;
    }

    touchdev = XOpenDevice(display, touch->id);
    if (!touchdev) {
        g_warning("Failed to open touch device \"%s\"", touch->name);
        return;
    }

    g_message("X11 Touch disabler active for device \"%s\"", touch->name);

    enabledAtom = XInternAtom(display, "Device Enabled", false);
}

void TouchDisableX11::enableTouch() {
    if (!touchdev) {
        return;
    }

    unsigned char value = 1;
    XChangeDeviceProperty(display, touchdev, enabledAtom, XA_INTEGER, 8, PropModeReplace, &value, 1);
    g_message("X11 Touch enabled");
}

void TouchDisableX11::disableTouch() {
    if (!touchdev) {
        return;
    }
    unsigned char value = 0;
    XChangeDeviceProperty(display, touchdev, enabledAtom, XA_INTEGER, 8, PropModeReplace, &value, 1);
    g_message("X11 Touch disabled");
}

#endif
