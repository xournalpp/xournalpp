/*
 * Xournal++
 *
 * Handles input to draw a spline consisting of linear and cubic spline segments
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

class SplineHandler : public InputHandler {
public:
	SplineHandler(XournalView* xournal, XojPageView* redrawable, const PageRef& page);
	virtual ~SplineHandler();

	void draw(cairo_t* cr);

	bool onMotionNotifyEvent(const PositionInputData& pos);
	void onButtonReleaseEvent(const PositionInputData& pos);
	void onButtonPressEvent(const PositionInputData& pos);
	void onButtonDoublePressEvent(const PositionInputData& pos);
	virtual bool onKeyEvent(GdkEventKey* event);

private:
	virtual void drawShape(Point& currentPoint, const PositionInputData& pos);

protected:	
	DocumentView view;
};