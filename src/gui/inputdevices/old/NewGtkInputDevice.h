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

class ScrollHandling;
class Settings;
class ToolHandler;
class InputSequence;

class NewGtkInputDevice : public AbstractInputDevice
{
public:
	NewGtkInputDevice(GtkWidget* widget, XournalView* view, ScrollHandling* scrollHandling);
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
	XournalView* getView();

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

	/**
	 * Handle Key Press event
	 */
	bool eventKeyPressHandler(GdkEventKey* event);

private:
	static bool eventCallback(GtkWidget* widget, GdkEvent* event, NewGtkInputDevice* self);

private:
	protected:
	/**
	 * Running input
	 */
	InputSequence* inputRunning = nullptr;

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

	/**
	 * Scrollbars
	 */
	ScrollHandling* scrollHandling;

	/**
	 * If touch drawing is not enabled, disable it
	 */
	bool ignoreTouch = false;
};
