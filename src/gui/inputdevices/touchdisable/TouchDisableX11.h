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

#ifdef __unix__
#define X11_ENABLED
#endif

#ifdef __APPLE__
#undef X11_ENABLED
#endif

#ifdef _WIN32
#undef X11_ENABLED
#endif

#ifdef _WIN64
#undef X11_ENABLED
#endif

#ifdef X11_ENABLED

#include "TouchDisableInterface.h"

#include "XournalType.h"

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/XInput.h>

class TouchDisableX11: public TouchDisableInterface
{
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
