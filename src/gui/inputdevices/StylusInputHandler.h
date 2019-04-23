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

class StylusInputHandler : public PenInputHandler
{
public:
	explicit StylusInputHandler(InputContext* inputContext);
	~StylusInputHandler();

	bool handleImpl(GdkEvent* event) override;
	bool changeTool(GdkEvent* event) override;
private:
	/*
	 * On some devices tapping the finger on the screen is matched on the pen if it also touches the surface.
	 * These taps are matched as GDK_ENTER_NOTIFY which we filter out using this flag.
	 */
	bool isInWidget = false;
};


