#include "ArrowHandler.h"

#include "gui/XournalView.h"
#include "control/Control.h"
#include "undo/InsertUndoAction.h"
#include <cmath>

ArrowHandler::ArrowHandler(XournalView* xournal, XojPageView* redrawable, PageRef page)
 : BaseStrokeHandler(xournal, redrawable, page)
{
	XOJ_INIT_TYPE(ArrowHandler);
}

ArrowHandler::~ArrowHandler()
{
	XOJ_CHECK_TYPE(ArrowHandler);

	XOJ_RELEASE_TYPE(ArrowHandler);
}

void ArrowHandler::drawShape(Point& c, bool shiftDown)
{
	int count = stroke->getPointCount();

	if (count < 1)
	{
		stroke->addPoint(c);
	}
	else
	{
		Point p = stroke->getPoint(0);

		if (count > 4)
		{
			// remove previous points
			stroke->deletePoint(4);
			stroke->deletePoint(3);
			stroke->deletePoint(2);
			stroke->deletePoint(1);
		}

		// We've now computed the line points for the arrow
		// so we just have to build the head

		// set up the size of the arrowhead to be 1/8 the length of arrow
		double dist = sqrt(pow(c.x - p.x, 2.0) + pow(c.y - p.y, 2.0)) / 8.0;

		double angle = atan2((c.y - p.y), (c.x - p.x));
		// an appropriate delta is Pi/3 radians for an arrow shape
		double delta = M_PI / 6.0;

		stroke->addPoint(c);
		stroke->addPoint(Point(c.x - dist * cos(angle + delta), c.y - dist * sin(angle + delta)));
		stroke->addPoint(c);
		stroke->addPoint(Point(c.x - dist * cos(angle - delta), c.y - dist * sin(angle - delta)));
	}
}
