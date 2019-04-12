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

class InputContext;

class TouchInputHandler : public AbstractInputHandler
{
public:
	explicit TouchInputHandler(InputContext* inputContext);
	bool handleImpl(GdkEvent* event) override;
};


