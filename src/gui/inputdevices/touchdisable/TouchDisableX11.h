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

#include <XournalType.h>

#include "TouchDisableInterface.h"

// Disabled at the moment, may cause issues
//#define X11_ENABLED

#ifdef WIN32
#undef X11_ENABLED
#endif
#ifdef __APPLE__
#undef X11_ENABLED
#endif

#ifdef X11_ENABLED

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/XInput.h>

class TouchDisableX11 : public TouchDisableInterface
{
public:
	TouchDisableX11();
	virtual ~TouchDisableX11();

public:
	virtual void enableTouch();
	virtual void disableTouch();
	virtual void init();

private:
	XOJ_TYPE_ATTRIB;

	/**
	 * X11 Display
	 */
	Display* display;

	/**
	 * Touch device ID
	 */
	Atom touchAtom;

	/**
	 * Touch device
	 */
	XDeviceInfo* touch;

	/**
	 * Touch device
	 */
	XDevice* touchdev;

	/**
	 * Enable flag
	 */
	Atom enabledAtom;
};

#endif
