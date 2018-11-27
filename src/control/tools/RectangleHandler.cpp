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

void RectangleHandler::drawShape(Point& c, bool shiftDown)
{
	int count = stroke->getPointCount();

	if (count < 1)
	{
		stroke->addPoint(c);
	}
	else if (shiftDown)
	{
		// Draw square if shift is pressed
		Point p = stroke->getPoint(0);

		stroke->deletePointsFrom(1);

		int size = MAX(ABS(c.x - p.x), ABS(c.y - p.y));

		if (c.x - p.x < 0 || c.y - p.y < 0)
		{
			size = -size;
		}

		stroke->addPoint(Point(p.x, p.y + size));
		stroke->addPoint(Point(p.x + size, p.y + size));
		stroke->addPoint(Point(p.x + size, p.y));
		stroke->addPoint(p);
	}
	else
	{
		Point p = stroke->getPoint(0);
		stroke->deletePointsFrom(1);

		stroke->addPoint(Point(c.x, p.y));
		stroke->addPoint(c);
		stroke->addPoint(Point(p.x, c.y));
		stroke->addPoint(p);
	}
}
