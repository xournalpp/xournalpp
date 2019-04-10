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
	 * Snap point to grid (if enabled - Alt key pressed will toggle)
	 */
	if ( pos.isAltDown() != xournal->getControl()->getSettings()->isSnapGrid())
	{
		snapToGrid(c.x,c.y);
	}

	if (!this->started) //initialize first point
	{
		this->startPoint = c;
		this->started = true;
	}
	else
	{
		double width = c.x - this->startPoint.x;
		double height = c.y - this->startPoint.y;
	
		if (pos.isShiftDown())	// make square
		{
			int signW = width>0?1:-1;
			int signH = height>0?1:-1;
			width = MAX( width*signW, height*signH) * signW;	
			height = (width * signW) * signH;
		}

		double diameterX, diameterY;
		int npts;
		double center_x;
		double center_y;
		double angle;
			
		// set resolution proportional to radius
		if( !pos.isControlDown())
		{
			diameterX = width;
			diameterY = height;
			npts = (int) (diameterX * 2.0);
			center_x = this->startPoint.x + width / 2.0;
			center_y = this->startPoint.y + height / 2.0;
			angle = 0;
		}
		else
		{	//control key down, draw centered at cursor
			diameterX = width*2.0;
			diameterY = height*2.0;
			npts = (int) (diameterX + diameterY);
			center_x = this->startPoint.x;
			center_y = this->startPoint.y;
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
			double xp = center_x + diameterX / 2.0 * cos((2 * M_PI * i) / npts + angle + M_PI);
			double yp = center_y + diameterY / 2.0 * sin((2 * M_PI * i) / npts + angle + M_PI);
			stroke->addPoint(Point(xp, yp));
		}
	}
	
}
