/*
 * Xournal++
 *
 * Handles input to draw circle
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "BaseStrokeHandler.h"

class CircleHandler : public BaseStrokeHandler
{
public:
	CircleHandler(XournalView* xournal, XojPageView* redrawable, PageRef page);
	virtual ~CircleHandler();

private:
	virtual void drawShape(Point& currentPoint, bool shiftDown);

private:
	XOJ_TYPE_ATTRIB;
};

