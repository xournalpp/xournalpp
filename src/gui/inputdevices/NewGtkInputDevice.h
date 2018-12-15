/*
 * Xournal++
 *
 * Base class for device input handling
 * This class uses the Example from GTK 3.24.x
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "AbstractInputDevice.h"

class NewGtkInputDevice : public AbstractInputDevice
{
public:
	NewGtkInputDevice(GtkWidget* widget, XournalView* view);
	virtual ~NewGtkInputDevice();

public:
	/**
	 * Initialize the input handling, set input events
	 */
	virtual void initWidget();

protected:
	/**
	 * Mouse / pen moved event
	 */
	virtual bool motionEvent(XojPageView* pageView, GdkEventMotion* event);

	/**
	 * Handle all GTK Events
	 */
	virtual bool eventHandler(GdkEvent* event);

	/**
	 * Touch event
	 */
	virtual bool touchEvent(GdkEventTouch* event);

protected:
	/**
	 * Read Pressure over GTK
	 */
	bool getPressureMultiplier(GdkEvent* event, double& presure);

	/**
	 * Read pressure and position of the pen, if a pen is active
	 */
	virtual void readPositionAndPressure(GdkEventMotion* event, double& x, double& y, double& pressure);

private:
	static bool event_cb(GtkWidget* widget, GdkEvent* event, NewGtkInputDevice* self);
	static bool touch_event_cb(GtkWidget* widget, GdkEventTouch* event, NewGtkInputDevice* self);

private:
	XOJ_TYPE_ATTRIB;

protected:
	/**
	 * Current ongoing pointer events
	 *
	 * GdkDevice -> InputSequence
	 */
	// TODO Rename
	GHashTable* pointerInputList;

	/**
	 * Current ongoing touch events
	 *
	 * GdkEventSequence -> InputSequence
	 */
	// TODO Rename
	GHashTable* touchInputList;
};
