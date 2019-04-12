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


#include "AbstractInputHandler.h"

class KeyboardInputHandler : public AbstractInputHandler
{
public:
	explicit KeyboardInputHandler(InputContext* inputContext);
	bool handleImpl(GdkEvent* event) override;
};


