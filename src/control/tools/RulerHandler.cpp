#include "RulerHandler.h"

#include "gui/XournalView.h"
#include "control/Control.h"
#include "undo/InsertUndoAction.h"
#include <cmath>

RulerHandler::RulerHandler(XournalView* xournal, XojPageView* redrawable, PageRef page)
 : BaseStrokeHandler(xournal, redrawable, page)
{
}

RulerHandler::~RulerHandler()
{
}


void RulerHandler::snapRotation(double& x, double& y)
{
	Point firstPoint = stroke->getPoint(0);

	//snap to a grid - get the angle of the points
	//if it's near 0, pi/4, 3pi/4, pi, or the negatives
	//within epsilon, fix it to that value.
	
	double dist = sqrt(pow(x - firstPoint.x, 2.0) + pow(y - firstPoint.y, 2.0));
	double angle = atan2((y - firstPoint.y), (x - firstPoint.x));
	double epsilon = xournal->getControl()->getSettings()->getSnapRotationTolerance();
	
	if (std::abs(angle) < epsilon)
	{
		x = dist + firstPoint.x;
		y = firstPoint.y;
	}
	else if (std::abs(angle - M_PI / 4.0) < epsilon)
	{
		x = dist / sqrt(2.0) + firstPoint.x;
		y = dist / sqrt(2.0) + firstPoint.y;
	}
	else if (std::abs(angle - 3.0 * M_PI / 4.0) < epsilon)
	{
		x = -dist / sqrt(2.0) + firstPoint.x;
		y = dist / sqrt(2.0) + firstPoint.y;
	}
	else if (std::abs(angle + M_PI / 4.0) < epsilon)
	{
		x = dist / sqrt(2.0) + firstPoint.x;
		y = -dist / sqrt(2.0) + firstPoint.y;
	}
	else if (std::abs(angle + 3.0 * M_PI / 4.0) < epsilon)
	{
		x = -dist / sqrt(2.0) + firstPoint.x;
		y = -dist / sqrt(2.0) + firstPoint.y;
	}
	else if (std::abs(std::abs(angle) - M_PI) < epsilon)
	{
		x = -dist + firstPoint.x;
		y = firstPoint.y;
	}
	else if (std::abs(angle - M_PI / 2.0) < epsilon)
	{
		x = firstPoint.x;
		y = dist + firstPoint.y;
	}
	else if (std::abs(angle + M_PI / 2.0) < epsilon)
	{
		x = firstPoint.x;
		y = -dist + firstPoint.y;
	}
}


void RulerHandler::drawShape(Point& currentPoint, const PositionInputData& pos)
{
	this->currPoint = currentPoint;	// in case redrawn by keypress event in base class.

	double x = currentPoint.x;
	double y = currentPoint.y;

	/**
	 * Snap first point to grid (if enabled)
	 */
	bool altDown = pos.isAltDown();
	if (!altDown && xournal->getControl()->getSettings()->isSnapGrid())
	{
		Point firstPoint = stroke->getPoint(0);
		snapToGrid(firstPoint.x,firstPoint.y);
		stroke->setFirstPoint(firstPoint.x,firstPoint.y);
	}


		
	if (stroke->getPointCount() < 2)
	{
		stroke->addPoint(currentPoint);
	}
	else if (altDown)
	{
		x = currentPoint.x;
		y = currentPoint.y;
	}
	else
	{
		if (xournal->getControl()->getSettings()->isSnapRotation())
		{
			snapRotation(x,y);
		}

		if (xournal->getControl()->getSettings()->isSnapGrid())
		{
			snapToGrid(x,y);
		}
	}

	stroke->setLastPoint(x, y);
}

