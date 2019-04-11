#include "RectangleHandler.h"

#include "gui/XournalView.h"
#include "control/Control.h"
#include "undo/InsertUndoAction.h"
#include <cmath>

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
	/**
	 * Snap point to grid (if enabled)
	 */
	if (pos.isAltDown() != xournal->getControl()->getSettings()->isSnapGrid())
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
	
		this->setModifiers(width, height,  pos);	// sets this->modShift and this->modControl
		
		if (this->modShift)	// make square
		{
			int signW = width>0?1:-1;
			int signH = height>0?1:-1;
			width = MAX( width*signW, height*signH) * signW;	
			height = (width * signW) * signH;
		}
		
		Point p1;
		if ( !this->modControl )
		{
			p1 = this->startPoint;
			
		}
		else	//Control is down - drawing from center
		{
			p1 = Point( this->startPoint.x - width, this->startPoint.y - height);
		}
		
		Point p2 = Point( this->startPoint.x + width, this->startPoint.y + height);
		
		stroke->deletePointsFrom(0);	//delete previous points
		
		stroke->addPoint(p1);
		stroke->addPoint(Point(p1.x, p2.y ));
		stroke->addPoint(p2);
		stroke->addPoint(Point(p2.x, p1.y));
		stroke->addPoint(p1);
	}

}
