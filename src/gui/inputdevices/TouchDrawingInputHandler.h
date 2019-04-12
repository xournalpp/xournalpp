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

#include "PenInputHandler.h"

class InputContext;

class TouchDrawingInputHandler : public PenInputHandler
{
private:
	GdkEventSequence* currentSequence = nullptr;
public:
	explicit TouchDrawingInputHandler(InputContext* inputContext);

	bool handleImpl(GdkEvent* event) override;
protected:
	bool changeTool(GdkEvent* event) override;
};


