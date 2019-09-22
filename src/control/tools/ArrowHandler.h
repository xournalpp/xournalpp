/*
 * Xournal++
 *
 * Handles input to draw an arrow
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "BaseStrokeHandler.h"

class ArrowHandler : public BaseStrokeHandler
{
public:
	ArrowHandler(XournalView* xournal, XojPageView* redrawable, PageRef page);
	virtual ~ArrowHandler();

private:
	virtual void drawShape(Point& currentPoint, const PositionInputData& pos);

private:
	};

