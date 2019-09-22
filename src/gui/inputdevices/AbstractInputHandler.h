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
	bool blocked = false;

protected:
	InputContext* inputContext;
	bool inputRunning = false;

protected:
	XojPageView* getPageAtCurrentPosition(InputEvent* event);
	PositionInputData getInputDataRelativeToCurrentPage(XojPageView* page, InputEvent* event);

public:
	explicit AbstractInputHandler(InputContext* inputContext);
	virtual ~AbstractInputHandler();

	void block(bool block);
	bool isBlocked();
	virtual void onBlock();
	bool handle(InputEvent* event);
	virtual bool handleImpl(InputEvent* event) = 0;
};


