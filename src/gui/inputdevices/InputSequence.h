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
#include "PositionInputData.h"

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
	bool actionMoved();

	/**
	 * Mouse / Pen down / touch start
	 */
	bool actionStart();

	/**
	 * Mouse / Pen up / touch end
	 */
	void actionEnd();

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
	void setButton(guint button);

	/**
	 * Set state flags from GDKevent (Shift down etc.)
	 */
	void setState(GdkModifierType state);

	/**
	 * Get Page at current position
	 *
	 * @return page or NULL if none
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
	XOJ_TYPE_ATTRIB;

	/**
	 * true if this input is running, false to ignore
	 */
	bool inputRunning;

	/**
	 * Input Handler
	 */
	NewGtkInputDevice* inputHandler;

	/**
	 * Current editing page
	 */
	XojPageView* current_view;

	/**
	 * Current input page. Mege together with current_view??
	 */
	XojPageView* currentInputPage;

	/**
	 * Current input device
	 */
	GdkDevice* device;

	/**
	 * This is a pen / eraser device
	 */
	bool penDevice;

	/**
	 * Axes of the input
	 */
	gdouble* axes;

	/**
	 * Pressure sensitivity enabled
	 */
	bool presureSensitivity;

	/**
	 * Current mouse button
	 */
	guint button;

	/**
	 * State flags from GDKevent (Shift down etc.)
	 */
	GdkModifierType state;

	/**
	 * Position X
	 */
	double x;

	/**
	 * Position Y
	 */
	double y;

	/**
	 * Root Position X
	 */
	double rootX;

	/**
	 * Root Position Y
	 */
	double rootY;

	/**
	 * Last mouse position for Scrolling
	 */
	int lastMousePositionX;

	/**
	 * Last mouse position for Scrolling
	 */
	int lastMousePositionY;

	/**
	 * Currently scrolling active
	 */
	bool inScrolling;

	/**
	 * The last Mouse Position, for scrolling
	 */
	int scrollOffsetX;

	/**
	 * The last Mouse Position, for scrolling
	 */
	int scrollOffsetY;
};
