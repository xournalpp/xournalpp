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

enum DIRSET_MODIFIERS
{
	NONE = 0,
	SET = 1,
	SHIFT = 1 << 1,
	CONTROL = 1 << 2
};


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
	virtual void drawShape(Point& currentPoint, const PositionInputData& pos) = 0;
	DIRSET_MODIFIERS drawModifierFixed = NONE;
	int lastCursor = -1;	//avoid same setCursor
	
protected:
	void snapToGrid(double& x, double& y);
	/**
	 * modifyModifiersByDrawDir  -  toggle shift and control modifiers depending on initial drawing direction/
	 */
	void modifyModifiersByDrawDir(double width, double height, bool changeCursor = true);
	bool modShift = false;
	bool modControl = false;

protected:
	XOJ_TYPE_ATTRIB;

	DocumentView view;
};

