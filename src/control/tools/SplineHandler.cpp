#include "SplineHandler.h"

#include "control/Control.h"
#include "control/layer/LayerController.h"
#include "gui/XournalView.h"
#include "gui/XournalppCursor.h"
#include "undo/InsertUndoAction.h"
#include "util/cpp14memory.h"

/**
 * @brief A class to handle splines
 * 
 * Drawing of a spline is started by a ButtonPressEvent. After every ButtonReleaseEvent, 
 * a new knot point is added. The spline is finished through a ButtonDoublePressEvent.
 * Splines segments can be linear or cubic (as in Inkscape). Join of two cubic segments
 * is supposed to be smooth.
 * As for now only linear splines are implemented.
 */
SplineHandler::SplineHandler(XournalView* xournal, XojPageView* redrawable, const PageRef& page)
 : InputHandler(xournal, redrawable, page) {
}

SplineHandler::~SplineHandler() = default;

void SplineHandler::draw(cairo_t* cr) {
	double zoom = xournal->getZoom();
	int dpiScaleFactor = xournal->getDpiScaleFactor();

	cairo_scale(cr, zoom * dpiScaleFactor, zoom * dpiScaleFactor);
	view.drawStroke(cr, stroke, 0);
}

auto SplineHandler::onKeyEvent(GdkEventKey* event) -> bool {
	return false;
}

auto SplineHandler::onMotionNotifyEvent(const PositionInputData& pos) -> bool {
	if (!stroke) {
		return false;
	}

	double zoom = xournal->getZoom();
	int pointCount = stroke->getPointCount();

	Point currentPoint = Point(pos.x / zoom, pos.y / zoom);
	Rectangle rect = stroke->boundingRect();

	if (pointCount > 0) {
		if (!validMotion(currentPoint, stroke->getPoint(pointCount - 1))) {
			return true;
		}
	}

	this->redrawable->repaintRect(stroke->getX(), stroke->getY(),
								  stroke->getElementWidth(), stroke->getElementHeight());

	drawShape(currentPoint, pos);
	
	rect.add(stroke->boundingRect());
	double w = stroke->getWidth();

	redrawable->repaintRect(rect.x - w, rect.y - w,
							rect.width + 2 * w,
							rect.height + 2 * w);

	return true;
}

void SplineHandler::onButtonReleaseEvent(const PositionInputData& pos) {
	double zoom = xournal->getZoom();
	if (stroke) {
		stroke->addPoint(Point(pos.x/zoom, pos.y/zoom)); 
	}
}

void SplineHandler::onButtonPressEvent(const PositionInputData& pos) {
	double zoom = xournal->getZoom();
	if (!stroke) {
		createStroke(Point(pos.x/zoom, pos.y/zoom));
	}
}

void SplineHandler::onButtonDoublePressEvent(const PositionInputData& pos)
{
	if (stroke == nullptr) {
		return;
	}	

	Control* control = xournal->getControl();

	// This is not a valid stroke
	if (stroke->getPointCount() < 2) {
		g_warning("Stroke incomplete!");
		delete stroke;
		stroke = nullptr;
		return;
	}

	stroke->freeUnusedPointItems();

	control->getLayerController()->ensureLayerExists(page);

	Layer* layer = page->getSelectedLayer();

	UndoRedoHandler* undo = control->getUndoRedoHandler();

	undo->addUndoAction(mem::make_unique<InsertUndoAction>(page, layer, stroke));

	layer->addElement(stroke);
	page->fireElementChanged(stroke);

	stroke = nullptr;

	xournal->getCursor()->updateCursor();
}

void SplineHandler::drawShape(Point& c, const PositionInputData& pos) {
	int count = stroke->getPointCount();
	if (count > 1) {
		// remove previous point
		stroke->deletePoint(count-1);
	}
	
	stroke->addPoint(c);
}