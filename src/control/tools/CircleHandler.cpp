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

void CircleHandler::drawShape(Point& c, const PositionInputData& pos)
{
	XOJ_CHECK_TYPE(CircleHandler);

	/**
	 * Snap first point to grid (if enabled)
	 */
	if (!pos.isShiftDown() && xournal->getControl()->getSettings()->isSnapGrid())
	{
		snapToGrid(c.x,c.y);
	}


	if (!this->started) //initialize circle
	{
		this->startPoint = c;
		this->started = true;
	}
	else
	{
		Point p = this->startPoint;
		if (xournal->getControl()->getSettings()->isSnapGrid())
		{
			snapToGrid(c.x,c.y);
		}
		
		double diameter;
		int npts;
		double center_x;
		double center_y;
		double angle;
			
		// set resolution proportional to radius
		if( !pos.isControlDown())
		{
			diameter = sqrt(pow(c.x - p.x, 2.0) + pow(c.y - p.y, 2.0));
			npts = (int) (diameter * 2.0);
			center_x = (c.x + p.x) / 2.0;
			center_y = (c.y + p.y) / 2.0;
			angle = atan2((c.y - p.y), (c.x - p.x));
		}
		else
		{	//control key down, draw centered at cursor
			diameter = sqrt(pow((c.x - p.x)*2, 2.0) + pow((c.y - p.y)*2, 2.0));
			npts = (int) (diameter * 2.0);
			center_x = p.x;
			center_y = p.y;
			angle = 0;
		}
		if (npts < 24)
		{
			npts = 24; // min. number of points
		}

		// remove previous points
		stroke->deletePointsFrom(0);
		for (int j = 0; j <= npts; j++)
		{
			int i = j%npts;	//so that we end exactly where we started.
			double xp = center_x + diameter / 2.0 * cos((2 * M_PI * i) / npts + angle + M_PI);
			double yp = center_y + diameter / 2.0 * sin((2 * M_PI * i) / npts + angle + M_PI);
			stroke->addPoint(Point(xp, yp));
		}
	}
}
