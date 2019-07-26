/*
 * Xournal++
 *
 * [Header description]
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <XournalType.h>
#include "AbstractInputHandler.h"

class InputContext;

class PenInputHandler : public AbstractInputHandler
{
private:
	XOJ_TYPE_ATTRIB;

	XojPageView* lastActivePage = nullptr;

protected:
	/**
	 * Whether the current device class has a device with button 1 in pressed state
	 */
	bool deviceClassPressed = false;
	/**
	 * modifier varaibles storing whether button 2 and 3 are pressed
	 */
	bool modifier2 = false;
	bool modifier3 = false;

	/**
	 * Reference to the last event
	 */
	InputEvent* lastEvent = nullptr;

	/**
	 * Reference to the last event actually hitting a page
	 */
	InputEvent* lastHitEvent = nullptr;

	/**
	 * Flag whether pen is within the widget
	 */
	bool penInWidget = false;

	/**
	 * Page a selection started at as we require this for motion updates
	 */
	 XojPageView* sequenceStartPage = nullptr;

public:
	explicit PenInputHandler(InputContext* inputContext);
	~PenInputHandler() override;

protected:
	/**
	 * Action for the start of an input
	 * @param event The event triggering the action
	 */
	bool actionStart(InputEvent* event);

	/**
	 * Action for motion during an input
	 * @param event The event triggering the action
	 */
	bool actionMotion(InputEvent* event);

	/**
	 * Action for a discrete input.
	 */
	void actionPerform(InputEvent* event);

	void actionLeaveWindow(InputEvent* event);
	void actionEnterWindow(InputEvent* event);

	/**
	 * Action for the end of an input
	 * @param event The event triggering the action
	 */
	bool actionEnd(InputEvent* event);

	virtual bool changeTool(InputEvent* event) = 0;

	/**
	 * Stores references to the provided event in lastEvent and lastHitEvent
	 * Use this to update the references for the last event.
	 * lastHitEvent will only be updated when the event did hit a page
	 * @param event
	 */
	void updateLastEvent(InputEvent* event);
};


