#include "RulerHandler.h"

#include "gui/XournalView.h"
#include "control/Control.h"
#include "undo/InsertUndoAction.h"
#include <cmath>

RulerHandler::RulerHandler(XournalView* xournal, XojPageView* redrawable, PageRef page)
 : BaseStrokeHandler(xournal, redrawable, page)
{
	XOJ_INIT_TYPE(RulerHandler);
}

RulerHandler::~RulerHandler()
{
	XOJ_CHECK_TYPE(RulerHandler);

	XOJ_RELEASE_TYPE(RulerHandler);
}

void RulerHandler::drawShape(Point& currentPoint)
{
	int count = stroke->getPointCount();

	if (count < 2)
	{
		stroke->addPoint(currentPoint);
	}
	else
	{
		double x = currentPoint.x;
		double y = currentPoint.y;

		//snap to a grid - get the angle of the points
		//if it's near 0, pi/4, 3pi/4, pi, or the negatives
		//within epsilon, fix it to that value.
		Point firstPoint = stroke->getPoint(0);

		double dist = sqrt(pow(x - firstPoint.x, 2.0) + pow(y - firstPoint.y, 2.0));
		double angle = atan2((y - firstPoint.y), (x - firstPoint.x));
		double epsilon = 0.1;
		if (std::abs(angle) < epsilon)
		{
			stroke->setLastPoint(dist + firstPoint.x, firstPoint.y);
		}
		else if (std::abs(angle - M_PI / 4.0) < epsilon)
		{
			stroke->setLastPoint(dist / sqrt(2.0) + firstPoint.x, dist / sqrt(2.0) + firstPoint.y);
		}
		else if (std::abs(angle - 3.0 * M_PI / 4.0) < epsilon)
		{
			stroke->setLastPoint(-dist / sqrt(2.0) + firstPoint.x, dist / sqrt(2.0) + firstPoint.y);
		}
		else if (std::abs(angle + M_PI / 4.0) < epsilon)
		{
			stroke->setLastPoint(dist / sqrt(2.0) + firstPoint.x, -dist / sqrt(2.0) + firstPoint.y);
		}
		else if (std::abs(angle + 3.0 * M_PI / 4.0) < epsilon)
		{
			stroke->setLastPoint(-dist / sqrt(2.0) + firstPoint.x, -dist / sqrt(2.0) + firstPoint.y);
		}
		else if (std::abs(std::abs(angle) - M_PI) < epsilon)
		{
			stroke->setLastPoint(-dist + firstPoint.x, firstPoint.y);
		}
		else if (std::abs(angle - M_PI / 2.0) < epsilon)
		{
			stroke->setLastPoint(firstPoint.x, dist + firstPoint.y);
		}
		else if (std::abs(angle + M_PI / 2.0) < epsilon)
		{
			stroke->setLastPoint(firstPoint.x, -dist + firstPoint.y);
		}
		else
		{
			stroke->setLastPoint(x, y);
		}
	}
}

