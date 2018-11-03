#include "RulerHandler.h"

#include "gui/XournalView.h"
#include "control/Control.h"
#include "undo/InsertUndoAction.h"

RulerHandler::RulerHandler(XournalView* xournal,
                           PageView* redrawable,
                           PageRef page)
	: InputHandler(xournal, redrawable, page)
{
	XOJ_INIT_TYPE(RulerHandler);
}

RulerHandler::~RulerHandler()
{
	XOJ_CHECK_TYPE(RulerHandler);

	XOJ_RELEASE_TYPE(RulerHandler);
}

void RulerHandler::draw(cairo_t* cr)
{
	XOJ_CHECK_TYPE(RulerHandler);

	double zoom = xournal->getZoom();
	cairo_scale(cr, zoom, zoom);

	view.drawStroke(cr, stroke, 0);
}

bool RulerHandler::onMotionNotifyEvent(GdkEventMotion* event)
{
	XOJ_CHECK_TYPE(RulerHandler);

	if (!stroke)
	{
		return false;
	}

	double zoom = xournal->getZoom();
	double x = event->x / zoom;
	double y = event->y / zoom;
	int pointCount = stroke->getPointCount();

	Point currentPoint(x, y);
	Rectangle rect = stroke->boundingRect();

	if (pointCount > 0)
	{
		if(!validMotion(currentPoint, stroke->getPoint(pointCount - 1)))
			return true;
	}

	if (pointCount < 2)
	{
		stroke->addPoint(currentPoint);
	}
	else
	{
		stroke->setLastPoint(currentPoint);
	}
	
	rect.add(stroke->boundingRect());
	double w = stroke->getWidth();

	redrawable->repaintRect(rect.x - w, rect.y - w,
	                        rect.width + 2*w,
	                        rect.height + 2*w);

	return true;
}

void RulerHandler::onButtonReleaseEvent(GdkEventButton* event)
{
	XOJ_CHECK_TYPE(RulerHandler);

	if (!stroke)
	{
		return;
	}

	// Backward compatibility and also easier to handle for me;-)
	// I cannot draw a line with one point, to draw a visible line I need two points,
	// twice the same Point is also OK
	if (stroke->getPointCount() == 1)
	{
		ArrayIterator<Point> it = stroke->pointIterator();
		if (it.hasNext())
		{
			stroke->addPoint(it.next());
		}
		// No pressure sensitivity
		stroke->clearPressure();
	}

	stroke->freeUnusedPointItems();

	if (page->getSelectedLayerId() < 1)
	{
		// This creates a layer if none exists
		page->getSelectedLayer();
		page->setSelectedLayerId(1);
		xournal->getControl()->getWindow()->updateLayerCombobox();
	}

	Layer* layer = page->getSelectedLayer();

	UndoRedoHandler* undo = xournal->getControl()->getUndoRedoHandler();

	undo->addUndoAction(new InsertUndoAction(page,
	                                         layer,
	                                         stroke));

	//TODO: Take care of the ShapeRecognizer

	layer->addElement(stroke);
	page->fireElementChanged(stroke);

	stroke = NULL;

	return;
}

void RulerHandler::onButtonPressEvent(GdkEventButton* event)
{
	XOJ_CHECK_TYPE(RulerHandler);

	double zoom = xournal->getZoom();
	double x = event->x / zoom;
	double y = event->y / zoom;

	if (!stroke)
	{
		createStroke(Point(x, y));
	}
}
