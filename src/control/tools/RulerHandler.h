/*
 * Xournal++
 *
 * Handles input of the ruler
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __RULERHANDLER_H__
#define __RULERHANDLER_H__

#include "InputHandler.h"

#include "../../view/DocumentView.h"

class RulerHandler : public InputHandler
{
public:
	RulerHandler(XournalView* xournal,
	             PageView* redrawable,
	             PageRef page);

	virtual ~RulerHandler();

	void draw(cairo_t* cr);

	bool onMotionNotifyEvent(GdkEventMotion* event);
	void onButtonReleaseEvent(GdkEventButton* event);
	void onButtonPressEvent(GdkEventButton* event);

private:
	XOJ_TYPE_ATTRIB;

	DocumentView view;
};

#endif
