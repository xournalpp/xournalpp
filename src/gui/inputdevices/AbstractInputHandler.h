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
	bool pressureSensitivity;
	bool inputRunning = false;

protected:
	XojPageView* getPageAtCurrentPosition(GdkEvent* event);
	PositionInputData getInputDataRelativeToCurrentPage(XojPageView* page, GdkEvent* event);

public:
	explicit AbstractInputHandler(InputContext* inputContext);
	virtual ~AbstractInputHandler();

	void block(bool block);
	bool isBlocked();
	virtual void onBlock();
	bool handle(GdkEvent* event);
	virtual bool handleImpl(GdkEvent* event) = 0;
};


