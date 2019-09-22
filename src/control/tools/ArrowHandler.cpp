#include "ArrowHandler.h"

#include "gui/XournalView.h"
#include "control/Control.h"
#include "undo/InsertUndoAction.h"
#include <cmath>

ArrowHandler::ArrowHandler(XournalView* xournal, XojPageView* redrawable, PageRef page)
 : BaseStrokeHandler(xournal, redrawable, page)
{
}

ArrowHandler::~ArrowHandler()
{
}

void ArrowHandler::drawShape(Point& c, const PositionInputData& pos)
{
	this->currPoint = c;	// in case redrawn by keypress event in base class.

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
	

	int count = stroke->getPointCount();
	if (count < 1)
	{
		stroke->addPoint(c);
	}
	else
	{
		Point p = stroke->getPoint(0);

		//disable this to see such a cool "serrate brush" effect
		if (count > 4)
		{
			// remove previous points
			stroke->deletePoint(4);
			stroke->deletePoint(3);
			stroke->deletePoint(2);
			stroke->deletePoint(1);
		}

		if (!altDown && xournal->getControl()->getSettings()->isSnapGrid())
		{
			snapToGrid(c.x,c.y);
		}

		// We've now computed the line points for the arrow
		// so we just have to build the head

		// set up the size of the arrowhead to be 1/8 the length of arrow
		double dist = sqrt(pow(c.x - p.x, 2.0) + pow(c.y - p.y, 2.0));
		double arrowDist = dist/8;

		double angle = atan2((c.y - p.y), (c.x - p.x));
		// an appropriate delta is Pi/3 radians for an arrow shape
		double delta = M_PI / 6.0;
		
		if (altDown || !xournal->getControl()->getSettings()->isSnapRotation())
		{
			stroke->addPoint(c);
			stroke->addPoint(Point(c.x - arrowDist * cos(angle + delta), c.y - arrowDist * sin(angle + delta)));
			stroke->addPoint(c);
			stroke->addPoint(Point(c.x - arrowDist * cos(angle - delta), c.y - arrowDist * sin(angle - delta)));
		}
		else
		{
			double epsilon = xournal->getControl()->getSettings()->getSnapRotationTolerance();
			if (std::abs(angle) < epsilon)
			{
				angle = 0;
				stroke->addPoint(Point(c.x, p.y));
				stroke->addPoint(Point(c.x - arrowDist * cos(angle + delta), p.y - arrowDist * sin(angle + delta)));
				stroke->addPoint(Point(c.x, p.y));
				stroke->addPoint(Point(c.x - arrowDist * cos(angle - delta), p.y - arrowDist * sin(angle - delta)));
			}
			else if (std::abs(angle - M_PI / 4.0) < epsilon)
			{
				angle = M_PI / 4.0;
				stroke->addPoint(Point(dist / sqrt(2.0) + p.x, dist / sqrt(2.0) + p.y));
				stroke->addPoint(Point(dist / sqrt(2.0) + p.x - arrowDist * cos(angle + delta), dist / sqrt(2.0) + p.y - arrowDist * sin(angle + delta)));
				stroke->addPoint(Point(dist / sqrt(2.0) + p.x, dist / sqrt(2.0) + p.y));
				stroke->addPoint(Point(dist / sqrt(2.0) + p.x - arrowDist * cos(angle - delta), dist / sqrt(2.0) + p.y - arrowDist * sin(angle - delta)));
			}
			else if (std::abs(angle - 3.0 * M_PI / 4.0) < epsilon)
			{
				angle = 3.0 * M_PI / 4.0;
				stroke->addPoint(Point(-dist / sqrt(2.0) + p.x, dist / sqrt(2.0) + p.y));
				stroke->addPoint(Point(-dist / sqrt(2.0) + p.x - arrowDist * cos(angle + delta), dist / sqrt(2.0) + p.y - arrowDist * sin(angle + delta)));
				stroke->addPoint(Point(-dist / sqrt(2.0) + p.x, dist / sqrt(2.0) + p.y));
				stroke->addPoint(Point(-dist / sqrt(2.0) + p.x - arrowDist * cos(angle - delta), dist / sqrt(2.0) + p.y - arrowDist * sin(angle - delta)));
			}
			else if (std::abs(angle + M_PI / 4.0) < epsilon)
			{
				angle = M_PI / 4.0;
				stroke->addPoint(Point(dist / sqrt(2.0) + p.x, -dist / sqrt(2.0) + p.y));
				stroke->addPoint(Point(dist / sqrt(2.0) + p.x - arrowDist * cos(angle + delta), -dist / sqrt(2.0) + p.y + arrowDist * sin(angle + delta)));
				stroke->addPoint(Point(dist / sqrt(2.0) + p.x, -dist / sqrt(2.0) + p.y));
				stroke->addPoint(Point(dist / sqrt(2.0) + p.x - arrowDist * cos(angle - delta), -dist / sqrt(2.0) + p.y + arrowDist * sin(angle - delta)));
			}
			else if (std::abs(angle + 3.0 * M_PI / 4.0) < epsilon)
			{
				angle = 3.0 * M_PI / 4.0;
				stroke->addPoint(Point(-dist / sqrt(2.0) + p.x, -dist / sqrt(2.0) + p.y));
				stroke->addPoint(Point(-dist / sqrt(2.0) + p.x - arrowDist * cos(angle + delta), -dist / sqrt(2.0) + p.y + arrowDist * sin(angle + delta)));
				stroke->addPoint(Point(-dist / sqrt(2.0) + p.x, -dist / sqrt(2.0) + p.y));
				stroke->addPoint(Point(-dist / sqrt(2.0) + p.x - arrowDist * cos(angle - delta), -dist / sqrt(2.0) + p.y + arrowDist * sin(angle - delta)));
			}
			else if (std::abs(std::abs(angle) - M_PI) < epsilon)
			{
				angle = - M_PI;
				stroke->addPoint(Point(c.x, p.y));
				stroke->addPoint(Point(c.x - arrowDist * cos(angle + delta), p.y - arrowDist * sin(angle + delta)));
				stroke->addPoint(Point(c.x, p.y));
				stroke->addPoint(Point(c.x - arrowDist * cos(angle - delta), p.y - arrowDist * sin(angle - delta)));
			}
			else if (std::abs(angle - M_PI / 2.0) < epsilon)
			{
				angle = M_PI / 2.0;
				stroke->addPoint(Point(p.x, c.y));
				stroke->addPoint(Point(p.x - arrowDist * cos(angle + delta), c.y - arrowDist * sin(angle + delta)));
				stroke->addPoint(Point(p.x, c.y));
				stroke->addPoint(Point(p.x - arrowDist * cos(angle - delta), c.y - arrowDist * sin(angle - delta)));
			}
			else if (std::abs(angle + M_PI / 2.0) < epsilon)
			{
				angle = - M_PI / 2.0;
				stroke->addPoint(Point(p.x, c.y));
				stroke->addPoint(Point(p.x - arrowDist * cos(angle + delta), c.y - arrowDist * sin(angle + delta)));
				stroke->addPoint(Point(p.x, c.y));
				stroke->addPoint(Point(p.x - arrowDist * cos(angle - delta), c.y - arrowDist * sin(angle - delta)));
			}
			else
			{
				stroke->addPoint(c);
				stroke->addPoint(Point(c.x - arrowDist * cos(angle + delta), c.y - arrowDist * sin(angle + delta)));
				stroke->addPoint(c);
				stroke->addPoint(Point(c.x - arrowDist * cos(angle - delta), c.y - arrowDist * sin(angle - delta)));
			}
		}
	}
}
