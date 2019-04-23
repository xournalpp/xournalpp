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
	XOJ_TYPE_ATTRIB;
public:
	explicit MouseInputHandler(InputContext* inputContext);
	~MouseInputHandler() override;

	bool handleImpl(GdkEvent* event) override;
	bool changeTool(GdkEvent* event) override;
	void onBlock() override;
};


