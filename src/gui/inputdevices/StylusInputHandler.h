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
#include "PenInputHandler.h"

class InputContext;

class StylusInputHandler : public PenInputHandler
{
public:
	explicit StylusInputHandler(InputContext* inputContext);
	~StylusInputHandler();

	bool handleImpl(InputEvent* event) override;
	bool changeTool(InputEvent* event) override;
private:
	void setPressedState(InputEvent*);
};


