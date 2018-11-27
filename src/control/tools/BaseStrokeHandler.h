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

	bool onMotionNotifyEvent(GdkEventMotion* event, bool shiftDown);
	void onButtonReleaseEvent(GdkEventButton* event);
	void onButtonPressEvent(GdkEventButton* event);

private:
	virtual void drawShape(Point& currentPoint, bool shiftDown) = 0;

protected:
	XOJ_TYPE_ATTRIB;

	DocumentView view;
};

