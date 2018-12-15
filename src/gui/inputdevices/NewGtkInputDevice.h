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
#include "gui/widgets/XournalWidget.h"

class Settings;
class ToolHandler;
class InputSequence;

class NewGtkInputDevice : public AbstractInputDevice
{
public:
	NewGtkInputDevice(GtkWidget* widget, XournalView* view);
	virtual ~NewGtkInputDevice();

public:
	/**
	 * Initialize the input handling, set input events
	 */
	void initWidget();

	/**
	 * Focus the widget
	 */
	void focusWidget();

	Settings* getSettings();
	ToolHandler* getToolHandler();
	GtkXournal* getXournal();

	/**
	 * Try to start input
	 *
	 * @return true if it should start
	 */
	bool startInput(InputSequence* input);

	/**
	 * Stop input of this sequence
	 */
	void stopInput(InputSequence* input);

protected:
	/**
	 * Handle all GTK Events
	 */
	bool eventHandler(GdkEvent* event);

private:
	static bool event_cb(GtkWidget* widget, GdkEvent* event, NewGtkInputDevice* self);

private:
	XOJ_TYPE_ATTRIB;

protected:
	/**
	 * Running input
	 */
	InputSequence* inputRunning;

	/**
	 * Current ongoing pointer events
	 *
	 * GdkDevice -> InputSequence
	 */
	GHashTable* pointerInputList;

	/**
	 * Current ongoing touch events
	 *
	 * GdkEventSequence -> InputSequence
	 */
	GHashTable* touchInputList;
};
