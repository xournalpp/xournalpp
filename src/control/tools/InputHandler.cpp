#include "InputHandler.h"
#include "../../gui/XournalWidget.h"
#include "../shaperecognizer/ShapeRecognizer.h"
#include "../Control.h"
#include "../../undo/InsertUndoAction.h"
#include "../../undo/RecognizerUndoAction.h"
#include <math.h>

#define PIXEL_MOTION_THRESHOLD 0.3

InputHandler::InputHandler(XournalWidget * xournal, GtkWidget * widget, Redrawable * redrawable) {
	this->tmpStroke = NULL;
	this->currentInputDevice = NULL;
	this->widget = widget;
	this->tmpStrokeDrawElem = 0;
	this->view = new DocumentView();
	this->redrawable = redrawable;
	this->xournal = xournal;
}

InputHandler::~InputHandler() {
	this->tmpStroke = NULL;
	this->currentInputDevice = NULL;
	this->redrawable = NULL;
	delete this->view;
}

void InputHandler::addPointToTmpStroke(GdkEventMotion * event) {
	double zoom = xournal->getZoom();
	double x = event->x / zoom;
	double y = event->y / zoom;

	if (tmpStroke->getPointCount() > 0) {
		Point p = tmpStroke->getPoint(tmpStroke->getPointCount() - 1);

		if (hypot(p.x - x, p.y - y) < PIXEL_MOTION_THRESHOLD) {
			return; // not a meaningful motion
		}
	}

	ToolHandler * h = xournal->getControl()->getToolHandler();

	if (h->isRuler()) {
		Range range(x, y);

		int count = tmpStroke->getPointCount();
		if (count < 2) {
			tmpStroke->addPoint(Point(x, y));
		} else {
			Point p = tmpStroke->getPoint(tmpStroke->getPointCount() - 1);
			range.addPoint(p.x, p.y);
			tmpStroke->setLastPoint(x, y);
		}
		Point p = tmpStroke->getPoint(0);
		range.addPoint(p.x, p.y);

		this->redrawable->repaint(range);
		return;
	}

	double pressure = Point::NO_PRESURE;
	if (h->getToolType() == TOOL_PEN) {
		if (getPressureMultiplier((GdkEvent *) event, pressure)) {
			pressure = pressure * tmpStroke->getWidth();
		} else {
			pressure = Point::NO_PRESURE;
		}
	}

	tmpStroke->setLastPressure(pressure);
	tmpStroke->addPoint(Point(x, y));

	drawTmpStroke();
}

bool InputHandler::getPressureMultiplier(GdkEvent *event, double & presure) {
	double * axes;
	GdkDevice * device;

	if (event->type == GDK_MOTION_NOTIFY) {
		axes = event->motion.axes;
		device = event->motion.device;
	} else {
		axes = event->button.axes;
		device = event->button.device;
	}

	if (device == gdk_device_get_core_pointer() || device->num_axes <= 2) {
		presure = 1.0;
		return false;
	}

	double rawpressure = axes[2] / (device->axes[2].max - device->axes[2].min);
	if (!finite(rawpressure)) {
		presure = 1.0;
		return false;
	}

	Settings * settings = xournal->getControl()->getSettings();

	presure = ((1 - rawpressure) * settings->getWidthMinimumMultiplier() + rawpressure
			* settings->getWidthMaximumMultiplier());
	return true;
}

void InputHandler::drawTmpStroke() {
	if (this->tmpStroke) {
		cairo_t * cr = gdk_cairo_create(widget->window);

		cairo_scale(cr, this->xournal->getZoom(), this->xournal->getZoom());

		view->drawStroke(cr, this->tmpStroke, tmpStrokeDrawElem);
		tmpStrokeDrawElem = this->tmpStroke->getPointCount() - 1;
		cairo_destroy(cr);
	}
}

void InputHandler::draw(cairo_t * cr) {
	if (this->tmpStroke) {
		view->drawStroke(cr, this->tmpStroke);
	}
}

void InputHandler::onButtonReleaseEvent(GdkEventButton * event, XojPage * page) {
	if(!this->tmpStroke) {
		return;
	}

	// Backward compatibility and also easier to handle for me;-)
	// I cannot draw a line with one point, to draw a visible line I need two points,
	// twice the same Point is also OK
	if (tmpStroke->getPointCount() == 1) {
		ArrayIterator<Point> it = tmpStroke->pointIterator();
		if (it.hasNext()) {
			tmpStroke->addPoint(it.next());
		}
		// No pressure sensitivity
		tmpStroke->clearPressure();
	}

	tmpStroke->freeUnusedPointItems();

	if (page->getSelectedLayerId() < 1) {
		// This creates a layer if none exists
		page->getSelectedLayer();
		page->setSelectedLayerId(1);
		xournal->getControl()->getWindow()->updateLayerCombobox();
	}

	Layer * layer = page->getSelectedLayer();

	UndoRedoHandler * undo = xournal->getControl()->getUndoRedoHandler();

	undo->addUndoAction(new InsertUndoAction(page, layer, tmpStroke, this->redrawable));

	ToolHandler * h = xournal->getControl()->getToolHandler();
	if (h->isShapeRecognizer()) {
		ShapeRecognizer reco;

		Stroke * s = reco.recognizePatterns(tmpStroke);

		if (s != NULL) {
			UndoRedoHandler * undo = xournal->getControl()->getUndoRedoHandler();
			undo->addUndoAction(new RecognizerUndoAction(page, this->redrawable, layer, tmpStroke, s));
			layer->addElement(s);

			Range range(s->getX(), s->getY());
			range.addPoint(s->getX() + s->getElementWidth(), s->getY() + s->getElementHeight());
			range.addPoint(tmpStroke->getX(), tmpStroke->getY());
			range.addPoint(tmpStroke->getX() + tmpStroke->getElementWidth(), tmpStroke->getY()
					+ tmpStroke->getElementHeight());

			this->redrawable->repaint(range);
		} else {
			layer->addElement(tmpStroke);
			this->redrawable->repaint(tmpStroke);
		}
	} else {
		layer->addElement(tmpStroke);
		this->redrawable->repaint(tmpStroke);
	}

	this->tmpStroke = NULL;

	if (currentInputDevice == event->device) {
		currentInputDevice = NULL;
	}

	this->tmpStrokeDrawElem = 0;
}

bool InputHandler::onMotionNotifyEvent(GdkEventMotion * event) {
	if (tmpStroke != NULL && this->currentInputDevice == event->device) {
		this->addPointToTmpStroke(event);
		return true;
	}
	return false;
}

void InputHandler::startStroke(GdkEventButton * event, StrokeTool tool, double x, double y) {
	ToolHandler * h = xournal->getControl()->getToolHandler();

	if (tmpStroke == NULL) {
		currentInputDevice = event->device;
		tmpStroke = new Stroke();
		tmpStroke->setWidth(h->getThikness());
		tmpStroke->setColor(h->getColor());
		tmpStroke->setToolType(tool);
		tmpStroke->addPoint(Point(x, y));
	}
}

Stroke * InputHandler::getTmpStroke() {
	return tmpStroke;
}

