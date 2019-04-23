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
public:
	explicit TouchInputHandler(InputContext* inputContext);
	~TouchInputHandler() override;

	bool handleImpl(GdkEvent* event) override;
};


