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

void RectangleHandler::drawShape(Point& c, const PositionInputData& pos)
{
	int count = stroke->getPointCount();

	/**
	 * Snap first point to grid (if enabled)
	 */
	if (xournal->getControl()->getSettings()->isSnapGrid())
	{
		Point firstPoint = stroke->getPoint(0);
		snapToGrid(firstPoint.x,firstPoint.y);
		stroke->setFirstPoint(firstPoint.x,firstPoint.y);
	}

	if (count < 1)
	{
		stroke->addPoint(c);
	}
	else if (pos.isShiftDown())
	{
		// Draw square if shift is pressed
		Point p = stroke->getPoint(0);

		stroke->deletePointsFrom(1);

		int size = MAX(ABS(c.x - p.x), ABS(c.y - p.y));
		int size_x, size_y;
		if (c.x - p.x < 0 )
		{
			size_x = -size;
			g_warning("size_x = -size");
		}
		else
		{
			size_x = size;
		}
		if  (c.y - p.y < 0)
		{
			size_y = -size;
			g_warning("size_y = -size");
		}
		else
		{
			size_y = size;
		}

		stroke->addPoint(Point(p.x, p.y + size_y));
		stroke->addPoint(Point(p.x + size_x, p.y + size_y));
		stroke->addPoint(Point(p.x + size_x, p.y));
		stroke->addPoint(p);
	}
	else
	{
		Point p = stroke->getPoint(0);
		stroke->deletePointsFrom(1);

		if (xournal->getControl()->getSettings()->isSnapGrid())
		{
			snapToGrid(c.x,c.y);
		}
		stroke->addPoint(Point(p.x, c.y));
		stroke->addPoint(c);
		stroke->addPoint(Point(c.x, p.y));
		stroke->addPoint(p);
	}

}
