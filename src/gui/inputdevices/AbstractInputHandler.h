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

#include <model/Point.h>
#include <gui/PageView.h>
#include <gui/inputdevices/old/PositionInputData.h>
#include <control/settings/ButtonConfig.h>

#include <gdk/gdk.h>

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
	bool pressureSensitivity;
	bool inputRunning = false;

protected:
	XojPageView* getPageAtCurrentPosition(GdkEvent* event);
	PositionInputData getInputDataRelativeToCurrentPage(XojPageView* page, GdkEvent* event);

public:
	explicit AbstractInputHandler(InputContext* inputContext);
	virtual ~AbstractInputHandler();

	void block(bool block);
	virtual void onBlock();
	bool handle(GdkEvent* event);
	virtual bool handleImpl(GdkEvent* event) = 0;
};


