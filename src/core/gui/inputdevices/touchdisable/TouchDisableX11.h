/*
 * Xournal++
 *
 * Interface for touch disable implementations
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#ifdef X11_ENABLED

#include <X11/X.h>                  // for Atom, None
#include <X11/Xlib.h>               // for Display
#include <X11/extensions/XInput.h>  // for XDevice, XDeviceInfo

#include "TouchDisableInterface.h"  // for TouchDisableInterface


class TouchDisableX11: public TouchDisableInterface {
public:
    TouchDisableX11();
    virtual ~TouchDisableX11();

public:
    virtual void enableTouch();
    virtual void disableTouch();
    virtual void init();

private:
    /**
     * X11 Display
     */
    Display* display = nullptr;

    /**
     * Touch device ID
     */
    Atom touchAtom = None;

    /**
     * Touch device
     */
    XDeviceInfo* touch = nullptr;

    /**
     * Touch device
     */
    XDevice* touchdev = nullptr;

    /**
     * Enable flag
     */
    Atom enabledAtom = None;
};

#endif
