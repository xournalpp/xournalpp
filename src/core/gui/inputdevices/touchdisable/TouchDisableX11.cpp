#include "TouchDisableX11.h"

#ifdef X11_ENABLED

#include <vector>  // for vector

#include <X11/Xatom.h>          // for XA_INTEGER
#include <X11/extensions/XI.h>  // for XI_TOUCHSCREEN
#include <gdk/gdk.h>            // for gdk_display_get_default
#include <gdk/gdkx.h>           // for gdk_x11_display_get_xdisplay
#include <glib.h>               // for g_message, g_warning, g_error

TouchDisableX11::TouchDisableX11() = default;

TouchDisableX11::~TouchDisableX11() {
    if (!touchdevs.empty()) {
        for (auto dev: touchdevs) {
            XCloseDevice(display, dev);
        }
    }
    touchdevs.clear();

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
    XDeviceInfo* devices = XListInputDevices(display, &inputDeviceCount);
    for (int i = 0; i < inputDeviceCount; i++) {
        if (devices[i].type == touchAtom) {
            const auto dev = XOpenDevice(display, devices[i].id);
            if (!dev) {
                g_warning("Failed to open touch device \"%s\"", devices[i].name);
                continue;
            }

            g_message("X11 Touch disabler active for device \"%s\"", devices[i].name);
            touchdevs.emplace_back(dev);
        }
    }
    XFreeDeviceList(devices);

    if (touchdevs.empty()) {
        g_warning("Could not find touchscreen device for disabling");
        return;
    }

    enabledAtom = XInternAtom(display, "Device Enabled", false);
}

void TouchDisableX11::enableTouch() {
    if (touchdevs.empty()) {
        return;
    }

    unsigned char value = 1;
    for (auto dev: touchdevs) {
        XChangeDeviceProperty(display, dev, enabledAtom, XA_INTEGER, 8, PropModeReplace, &value, 1);
    }
    g_message("X11 Touch enabled");
}

void TouchDisableX11::disableTouch() {
    if (touchdevs.empty()) {
        return;
    }

    unsigned char value = 0;
    for (auto dev: touchdevs) {
        XChangeDeviceProperty(display, dev, enabledAtom, XA_INTEGER, 8, PropModeReplace, &value, 1);
    }
    g_message("X11 Touch disabled");
}

#endif
