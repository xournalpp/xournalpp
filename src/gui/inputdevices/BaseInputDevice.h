/*
 * Xournal++
 *
 * Base class for device input handling
 * This class uses Standard GTK Functionality without any hacks
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "AbstractInputDevice.h"

class BaseInputDevice : public AbstractInputDevice
{
public:
	BaseInputDevice(GtkWidget* widget, XournalView* view);
	virtual ~BaseInputDevice();

public:
	/**
	 * Initialize the input handling, set input events
	 */
	virtual void initWidget();

protected:
	/**
	 * Mouse / pen moved event, handle pressure
	 */
	virtual bool motionEvent(XojPageView* pageView, GdkEventMotion* event);

	// GTK Event handling
public:
	/**
	 * Button pressed event
	 */
	virtual bool buttonPressEvent(GdkEventButton* event);

	/**
	 * Button release event
	 */
	virtual bool buttonReleaseEvent(GdkEventButton* event);

	/**
	 * Moved event
	 */
	virtual bool motionNotifyEvent(GdkEventMotion* event);

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

	// Static GTK Callbacks
private:
	static bool button_press_event_cb(GtkWidget* widget, GdkEventButton* event, BaseInputDevice* self);
	static bool button_release_event_cb(GtkWidget* widget, GdkEventButton* event, BaseInputDevice* self);
	static bool motion_notify_event_cb(GtkWidget* widget, GdkEventMotion* event, BaseInputDevice* self);
	static bool touch_event_cb(GtkWidget* widget, GdkEventTouch* event, BaseInputDevice* self);

private:
	XOJ_TYPE_ATTRIB;
};
