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

#include <XournalType.h>

#include <gtk/gtk.h>

class XournalView;
class XojPageView;

class BaseInputDevice
{
public:
	BaseInputDevice(GtkWidget* widget, XournalView* view);
	virtual ~BaseInputDevice();

public:
	/**
	 * Initialize the input handling, set input events
	 */
	virtual void initWidget();

	/**
	 * Mouse / pen moved event
	 */
	virtual bool motionEvent(XojPageView* pageView, GdkEventMotion* event);

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
	XOJ_TYPE_ATTRIB;

protected:

	/**
	 * Xournal Widget
	 */
	GtkWidget* widget;

	/**
	 * Xournal View
	 */
	XournalView* view;
};
