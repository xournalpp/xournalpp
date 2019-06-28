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
	double lastPosX = -1.0;
	double lastPosY = -1.0;

private:
	void actionStart(InputEvent* event);
	void actionMotion(InputEvent* event);
	void actionEnd(InputEvent* event);

public:
	explicit TouchInputHandler(InputContext* inputContext);
	~TouchInputHandler() override;

	bool handleImpl(InputEvent* event) override;

};


