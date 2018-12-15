#include "BaseStrokeHandler.h"

#include "gui/XournalView.h"
#include "control/Control.h"
#include "undo/InsertUndoAction.h"

BaseStrokeHandler::BaseStrokeHandler(XournalView* xournal, XojPageView* redrawable, PageRef page)
 : InputHandler(xournal, redrawable, page)
{
	XOJ_INIT_TYPE(BaseStrokeHandler);
}

BaseStrokeHandler::~BaseStrokeHandler()
{
	XOJ_CHECK_TYPE(BaseStrokeHandler);

	XOJ_RELEASE_TYPE(BaseStrokeHandler);
}

void BaseStrokeHandler::draw(cairo_t* cr)
{
	XOJ_CHECK_TYPE(BaseStrokeHandler);

	double zoom = xournal->getZoom();
	cairo_scale(cr, zoom, zoom);

	view.drawStroke(cr, stroke, 0);
}

bool BaseStrokeHandler::onMotionNotifyEvent(const PositionInputData& pos)
{
	XOJ_CHECK_TYPE(BaseStrokeHandler);

	if (!stroke)
	{
		return false;
	}

	double zoom = xournal->getZoom();
	double x = pos.x / zoom;
	double y = pos.y / zoom;
	int pointCount = stroke->getPointCount();

	Point currentPoint(x, y);
	Rectangle rect = stroke->boundingRect();

	if (pointCount > 0)
	{
		if (!validMotion(currentPoint, stroke->getPoint(pointCount - 1)))
		{
			return true;
		}
	}

	this->redrawable->repaintRect(stroke->getX(), stroke->getY(),
			stroke->getElementWidth(), stroke->getElementHeight());

	drawShape(currentPoint, pos.isShiftDown());
	
	rect.add(stroke->boundingRect());
	double w = stroke->getWidth();

	redrawable->repaintRect(rect.x - w, rect.y - w,
	                        rect.width + 2*w,
	                        rect.height + 2*w);

	return true;
}

void BaseStrokeHandler::onButtonReleaseEvent(const PositionInputData& pos)
{
	XOJ_CHECK_TYPE(BaseStrokeHandler);

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

	undo->addUndoAction(new InsertUndoAction(page, layer, stroke));

	//TODO: Take care of the ShapeRecognizer

	layer->addElement(stroke);
	page->fireElementChanged(stroke);

	stroke = NULL;

	return;
}

void BaseStrokeHandler::onButtonPressEvent(const PositionInputData& pos)
{
	XOJ_CHECK_TYPE(BaseStrokeHandler);

	double zoom = xournal->getZoom();
	double x = pos.x / zoom;
	double y = pos.y / zoom;

	if (!stroke)
	{
		createStroke(Point(x, y));
	}
}
