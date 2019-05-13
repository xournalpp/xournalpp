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

class TouchInputHandler : public AbstractInputHandler
{
private:
	XOJ_TYPE_ATTRIB;
	GdkEventSequence* currentSequence = nullptr;
	double lastPosX = 0.0;
	double lastPosY = 0.0;

private:
	void actionStart(GdkEvent* event);
	void actionMotion(GdkEvent* event);
	void actionEnd(GdkEvent* event);

public:
	explicit TouchInputHandler(InputContext* inputContext);
	~TouchInputHandler() override;

	bool handleImpl(GdkEvent* event) override;

};


