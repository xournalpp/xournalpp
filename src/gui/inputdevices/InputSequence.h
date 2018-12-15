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

class InputSequence
{
public:
	InputSequence();
	virtual ~InputSequence();

public:
	/**
	 * End / finalize input
	 */
	void endInput();

	/**
	 * All data applied, do the input now
	 */
	void handleInput();

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
	 * Set Position
	 */
	void setCurrentPosition(double x, double y);

public:
	/**
	 * Free an input sequence, used as callback for GTK
	 */
	static void free(InputSequence* sequence);

private:
	XOJ_TYPE_ATTRIB;

	/**
	 * Current input device
	 */
	GdkDevice* device;

	/**
	 * Axes of the input
	 */
	gdouble* axes;

	/**
	 * Position X
	 */
	double x;

	/**
	 * Position Y
	 */
	double y;
};
