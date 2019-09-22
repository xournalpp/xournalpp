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

class MouseInputHandler : public PenInputHandler
{
private:
	void setPressedState(InputEvent*);
public:
	explicit MouseInputHandler(InputContext* inputContext);
	~MouseInputHandler() override;

	bool handleImpl(InputEvent* event) override;
	bool changeTool(InputEvent* event) override;
	void onBlock() override;
};


