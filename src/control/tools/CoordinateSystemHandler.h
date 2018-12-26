/*
 * Xournal++
 *
 * Handles input to draw an coordinate system
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "BaseStrokeHandler.h"

class CoordinateSystemHandler : public BaseStrokeHandler
{
public:
	CoordinateSystemHandler(XournalView* xournal, XojPageView* redrawable, PageRef page);
	virtual ~CoordinateSystemHandler();

private:
	virtual void drawShape(Point& currentPoint, bool shiftDown);

private:
	XOJ_TYPE_ATTRIB;
};

