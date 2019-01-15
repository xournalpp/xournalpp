#include "CircleHandler.h"

#include "gui/XournalView.h"
#include "control/Control.h"
#include "undo/InsertUndoAction.h"
#include <cmath>

CircleHandler::CircleHandler(XournalView* xournal, XojPageView* redrawable, PageRef page)
	: BaseStrokeHandler(xournal, redrawable, page)
{
	XOJ_INIT_TYPE(CircleHandler);
}

CircleHandler::~CircleHandler()
{
	XOJ_CHECK_TYPE(CircleHandler);

	XOJ_RELEASE_TYPE(CircleHandler);
}

void CircleHandler::drawShape(Point& c, bool shiftDown)
{
	/**
	 * Snap first point to grid (if enabled)
	 */
	if (!shiftDown && xournal->getControl()->getSettings()->isSnapGrid())
	{
		Point firstPoint = stroke->getPoint(0);
		snapToGrid(firstPoint.x,firstPoint.y);
		stroke->setFirstPoint(firstPoint.x,firstPoint.y);
	}

	int count = stroke->getPointCount();

	if (count < 2)
	{
		stroke->addPoint(c);
	}
	else
	{
		Point p = stroke->getPoint(0);
		if (xournal->getControl()->getSettings()->isSnapGrid())
		{
			snapToGrid(c.x,c.y);
		}
		// set resolution proportional to radius
		double diameter = sqrt(pow(c.x - p.x, 2.0) + pow(c.y - p.y, 2.0));
		int npts = (int) (diameter * 2.0);
		double center_x = (c.x + p.x) / 2.0;
		double center_y = (c.y + p.y) / 2.0;
		double angle = atan2((c.y - p.y), (c.x - p.x));

		if (npts < 24)
		{
			npts = 24; // min. number of points
		}

		// remove previous points
		stroke->deletePointsFrom(1);
		for (int i = 1; i < npts; i++)
		{
			double xp = center_x + diameter / 2.0 * cos((2 * M_PI * i) / npts + angle + M_PI);
			double yp = center_y + diameter / 2.0 * sin((2 * M_PI * i) / npts + angle + M_PI);
			stroke->addPoint(Point(xp, yp));
		}
		stroke->addPoint(p);
	}
}
