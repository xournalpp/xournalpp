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

#include "AbstractInputHandler.h"

class InputContext;

class PenInputHandler : public AbstractInputHandler
{
private:
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
	GdkEvent* lastEvent = nullptr;

	/**
	 * Reference to the last event actually hitting a page
	 */
	GdkEvent* lastHitEvent = nullptr;

	/**
	 * Start position to reference scroll offset
	 */
	double scrollStartX = 0;
	double scrollStartY = 0;

	/**
	 * The offset to scroll in x-direction
	 */
	double scrollOffsetX = 0;

	/**
	 * The offset to scroll in y-direction
	 */
	double scrollOffsetY = 0;

public:
	explicit PenInputHandler(InputContext* inputContext);
	virtual ~PenInputHandler();

protected:
	/**
	 * Action for the start of an input
	 * @param event The event triggering the action
	 */
	bool actionStart(GdkEvent* event);

	/**
	 * Action for motion during an input
	 * @param event The event triggering the action
	 */
	bool actionMotion(GdkEvent* event);

	void actionLeaveWindow(GdkEvent* event);
	void actionEnterWindow(GdkEvent* event);

	/**
	 * Action for the end of an input
	 * @param event The event triggering the action
	 */
	bool actionEnd(GdkEvent* event);

	void setPressedState(GdkEvent*);

	virtual bool changeTool(GdkEvent* event) = 0;

	/**
	 * Do the scrolling with the hand tool
	 */
	void handleScrollEvent(GdkEvent* event);

	/**
	 * Stores references to the provided event in lastEvent and lastHitEvent
	 * Use this to update the references for the last event.
	 * lastHitEvent will only be updated when the event did hit a page
	 * @param event
	 */
	void updateLastEvent(GdkEvent* event);
};


