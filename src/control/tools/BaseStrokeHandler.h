/*
 * Xournal++
 *
 * Handles input of the ruler
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "InputHandler.h"

#include "model/Point.h"
#include "view/DocumentView.h"

class BaseStrokeHandler : public InputHandler
{
public:
	BaseStrokeHandler(XournalView* xournal, XojPageView* redrawable, PageRef page);

	virtual ~BaseStrokeHandler();

	void draw(cairo_t* cr);

	bool onMotionNotifyEvent(const PositionInputData& pos);
	void onButtonReleaseEvent(const PositionInputData& pos);
	void onButtonPressEvent(const PositionInputData& pos);

private:
	virtual void drawShape(Point& currentPoint, bool shiftDown) = 0;

protected:
	XOJ_TYPE_ATTRIB;
    void snapToGrid(double& x, double& y);
	DocumentView view;
};

