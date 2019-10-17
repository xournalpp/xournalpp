/*
 * Xournal++
 *
 * Base class for device input handling
 * This is an input sequence, like drawing a shape etc.
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <XournalType.h>

#include <gtk/gtk.h>
#include "gui/inputdevices/PositionInputData.h"

class NewGtkInputDevice;
class XojPageView;

class InputSequence
{
public:
	InputSequence(NewGtkInputDevice* inputHandler);
	virtual ~InputSequence();

public:

	/**
	 * Mouse / Pen / Touch move
	 */
	bool actionMoved(guint32 time);

	/**
	 * Mouse / Pen down / touch start
	 */
	bool actionStart(guint32 time);

	/**
	 * Mouse / Pen up / touch end
	 */
	void actionEnd(guint32 time);

	/**
	 * Check if input is still running, or if there an event was missed
	 *
	 * @return true if input is stopped now
	 */
	bool checkStillRunning();

	/**
	 * Set current input device
	 */
	void setDevice(GdkDevice* device);

	/**
	 * Clear the last stored axes
	 */
	void clearAxes();

	/**
	 * Set the axes
	 *
	 * @param axes Will be handed over, and freed by InputSequence
	 */
	void setAxes(gdouble* axes);

	/**
	 * Copy axes from event
	 */
	void copyAxes(GdkEvent* event);

	/**
	 * Set Position
	 */
	void setCurrentPosition(double x, double y);

	/**
	 * Set Root Position
	 */
	void setCurrentRootPosition(double x, double y);

	/**
	 * Set (mouse)button
	 */
	void setButton(guint button, guint time);

	/**
	 * Set state flags from GDKevent (Shift down etc.)
	 */
	void setState(GdkModifierType state);

	/**
	 * Get Page at current position
	 *
	 * @return page or nullptr if none
	 */
	XojPageView* getPageAtCurrentPosition();

public:
	/**
	 * Free an input sequence, used as callback for GTK
	 */
	static void free(InputSequence* sequence);

private:
	/**
	 * Change the tool according to the device and button
	 * @return true to ignore event
	 */
	bool changeTool();

	/**
	 * Check if this input can be started (don't do two inputs at the same time)
	 */
	void checkCanStartInput();

	/**
	 * Stop the running input, if running
	 */
	void stopInput();

	/**
	 * Do the scrolling with the hand tool
	 */
	void handleScrollEvent();

	/**
	 * Get input data relative to current input page
	 */
	PositionInputData getInputDataRelativeToCurrentPage(XojPageView* page);

private:
	/**
	 * true if this input is running, false to ignore
	 */
	bool inputRunning = false;

	/**
	 * Input Handler
	 */
	NewGtkInputDevice* inputHandler;

	/**
	 * Current editing page
	 */
	XojPageView* current_view = nullptr;

	/**
	 * Current input page. Mege together with current_view??
	 */
	XojPageView* currentInputPage = nullptr;

	/**
	 * Current input device
	 */
	GdkDevice* device = nullptr;

	/**
	 * This is a pen / eraser device
	 */
	bool penDevice = false;

	/**
	 * Axes of the input
	 */
	gdouble* axes = nullptr;

	/**
	 * Pressure sensitivity enabled
	 */
	bool presureSensitivity = false;

	/**
	 * Current mouse button
	 */
	guint button = 0;

	/**
	 * State flags from GDKevent (Shift down etc.)
	 */
	GdkModifierType state = (GdkModifierType)0;

	/**
	 * Position X
	 */
	double x = -1;

	/**
	 * Position Y
	 */
	double y = -1;

	/**
	 * Root Position X
	 */
	double rootX = 0;

	/**
	 * Root Position Y
	 */
	double rootY = 0;

	/**
	 * Last mouse position for Scrolling
	 */
	double lastMousePositionX = 0;

	/**
	 * Last mouse position for Scrolling
	 */
	double lastMousePositionY = 0;

	/**
	 * Currently scrolling active
	 */
	bool inScrolling = false;

	/**
	 * The last Mouse Position, for scrolling
	 */
	double scrollOffsetX = 0;

	/**
	 * The last Mouse Position, for scrolling
	 */
	double scrollOffsetY = 0;

	
	/**
	 * event time
	 */
	guint32 eventTime;
	
	
};
