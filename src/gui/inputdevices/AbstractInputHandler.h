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

#include <model/Point.h>
#include <gui/PageView.h>
#include <gui/inputdevices/PositionInputData.h>
#include <control/settings/ButtonConfig.h>

#include <gdk/gdk.h>
#include "InputEvents.h"

class InputContext;

/**
 * Abstract class for a specific input state
 */
class AbstractInputHandler
{
private:
	XOJ_TYPE_ATTRIB;

	bool blocked = false;

protected:
	InputContext* inputContext;
	bool inputRunning = false;

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

protected:
	XojPageView* getPageAtCurrentPosition(InputEvent* event);
	PositionInputData getInputDataRelativeToCurrentPage(XojPageView* page, InputEvent* event);

    /**
     * Do the scrolling with the hand tool
     */
    void handleScrollEvent(InputEvent* event);

public:
	explicit AbstractInputHandler(InputContext* inputContext);
	virtual ~AbstractInputHandler();

	void block(bool block);
	bool isBlocked();
	virtual void onBlock();
	bool handle(InputEvent* event);
	virtual bool handleImpl(InputEvent* event) = 0;
};


