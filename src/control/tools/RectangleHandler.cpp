#include "RectangleHandler.h"

#include "gui/XournalView.h"
#include "control/Control.h"
#include "undo/InsertUndoAction.h"

RectangleHandler::RectangleHandler(XournalView* xournal, XojPageView* redrawable, PageRef page)
 : BaseStrokeHandler(xournal, redrawable, page)
{
	XOJ_INIT_TYPE(RectangleHandler);
}

RectangleHandler::~RectangleHandler()
{
	XOJ_CHECK_TYPE(RectangleHandler);

	XOJ_RELEASE_TYPE(RectangleHandler);
}

void RectangleHandler::drawShape(Point& c)
{
	int count = stroke->getPointCount();

	if (count < 1)
	{
		stroke->addPoint(c);
	}
	else
	{
		Point p = stroke->getPoint(0);
		if (count > 3)
		{
			stroke->deletePoint(4);
			stroke->deletePoint(3);
			stroke->deletePoint(2);
			stroke->deletePoint(1);
		}
		stroke->addPoint(Point(c.x, p.y));
		stroke->addPoint(c);
		stroke->addPoint(Point(p.x, c.y));
		stroke->addPoint(p);
	}
}
