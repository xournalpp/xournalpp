/*
 * Xournal++
 *
 * Base class for device input handling
 * This is the interface / basis class for all implementations
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

class AbstractInputDevice
{
public:
	AbstractInputDevice(GtkWidget* widget, XournalView* view);
	virtual ~AbstractInputDevice();

public:
	/**
	 * Initialize the input handling, set input events
	 */
	virtual void initWidget() = 0;

private:
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
