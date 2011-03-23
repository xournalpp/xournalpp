#include "InputHandler.h"
#include "../../gui/XournalView.h"
#include "../../gui/PageView.h"
#include "../../gui/widgets/XournalWidget.h"
#include "../Control.h"
#include "../shaperecognizer/ShapeRecognizerResult.h"
#include "../../undo/InsertUndoAction.h"
#include "../../undo/RecognizerUndoAction.h"
#include <math.h>
#include "../../view/DocumentView.h"

#define PIXEL_MOTION_THRESHOLD 0.3

InputHandler::InputHandler(XournalView * xournal, PageView * redrawable) {
	XOJ_INIT_TYPE(InputHandler);

	this->tmpStroke = NULL;
	this->currentInputDevice = NULL;
	this->tmpStrokeDrawElem = 0;
	this->view = new DocumentView();
	this->redrawable = redrawable;
	this->xournal = xournal;
	this->reco = NULL;
}

InputHandler::~InputHandler() {
	XOJ_CHECK_TYPE(InputHandler);

	this->tmpStroke = NULL;
	this->currentInputDevice = NULL;
	this->redrawable = NULL;
	delete this->view;
	this->view = NULL;

	XOJ_RELEASE_TYPE(InputHandler);
}

void InputHandler::addPointToTmpStroke(GdkEventMotion * event) {
	XOJ_CHECK_TYPE(InputHandler);

	double zoom = xournal->getZoom();
	double x = event->x / zoom;
	double y = event->y / zoom;
	bool presureSensitivity = xournal->getControl()->getSettings()->isPresureSensitivity();

	if (tmpStroke->getPointCount() > 0) {
		Point p = tmpStroke->getPoint(tmpStroke->getPointCount() - 1);

		if (hypot(p.x - x, p.y - y) < PIXEL_MOTION_THRESHOLD) {
			return; // not a meaningful motion
		}
	}

	ToolHandler * h = xournal->getControl()->getToolHandler();

	if (h->isRuler()) {
		int count = tmpStroke->getPointCount();
		if (count < 2) {
			tmpStroke->addPoint(Point(x, y));
		} else {
			tmpStroke->setLastPoint(x, y);
		}
		Point p = tmpStroke->getPoint(0);

		this->redrawable->rerenderElement(this->tmpStroke);
		return;
	}

	if (presureSensitivity) {
		double pressure = Point::NO_PRESURE;
		if (h->getToolType() == TOOL_PEN) {
			if (getPressureMultiplier((GdkEvent *) event, pressure)) {
				pressure = pressure * tmpStroke->getWidth();
			} else {
				pressure = Point::NO_PRESURE;
			}
		}

		tmpStroke->setLastPressure(pressure);
	}
	tmpStroke->addPoint(Point(x, y));

	drawTmpStroke();
}

bool InputHandler::getPressureMultiplier(GdkEvent * event, double & presure) {
	XOJ_CHECK_TYPE(InputHandler);

	double * axes = NULL;
	GdkDevice * device = NULL;

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

	presure = ((1 - rawpressure) * settings->getWidthMinimumMultiplier() + rawpressure * settings->getWidthMaximumMultiplier());
	return true;
}

void InputHandler::drawTmpStroke() {
	XOJ_CHECK_TYPE(InputHandler);

	if (this->tmpStroke) {
		cairo_t * cr = gtk_xournal_create_cairo_for(this->xournal->getWidget(), this->redrawable);

		this->view->drawStroke(cr, this->tmpStroke, this->tmpStrokeDrawElem);
		this->tmpStrokeDrawElem = this->tmpStroke->getPointCount() - 1;
		cairo_destroy(cr);
	}
}

void InputHandler::draw(cairo_t * cr, double zoom) {
	XOJ_CHECK_TYPE(InputHandler);

	if (this->tmpStroke) {
		this->view->drawStroke(cr, this->tmpStroke, zoom);
	}
}

void InputHandler::onButtonReleaseEvent(GdkEventButton * event, XojPage * page) {
	XOJ_CHECK_TYPE(InputHandler);

	if (!this->tmpStroke) {
		return;
	}

	// Backward compatibility and also easier to handle for me;-)
	// I cannot draw a line with one point, to draw a visible line I need two points,
	// twice the same Point is also OK
	if (this->tmpStroke->getPointCount() == 1) {
		ArrayIterator<Point> it = this->tmpStroke->pointIterator();
		if (it.hasNext()) {
			this->tmpStroke->addPoint(it.next());
		}
		// No pressure sensitivity
		this->tmpStroke->clearPressure();
	}

	this->tmpStroke->freeUnusedPointItems();

	if (page->getSelectedLayerId() < 1) {
		// This creates a layer if none exists
		page->getSelectedLayer();
		page->setSelectedLayerId(1);
		xournal->getControl()->getWindow()->updateLayerCombobox();
	}

	Layer * layer = page->getSelectedLayer();

	UndoRedoHandler * undo = xournal->getControl()->getUndoRedoHandler();

	undo->addUndoAction(new InsertUndoAction(page, layer, this->tmpStroke, this->redrawable));

	ToolHandler * h = xournal->getControl()->getToolHandler();
	if (h->isShapeRecognizer()) {
		if (this->reco == NULL) {
			this->reco = new ShapeRecognizer();
		}
		ShapeRecognizerResult * result = this->reco->recognizePatterns(this->tmpStroke);

		if (result != NULL) {
			UndoRedoHandler * undo = xournal->getControl()->getUndoRedoHandler();

			Stroke * recognized = result->getRecognized();

			RecognizerUndoAction * recognizerUndo = new RecognizerUndoAction(page, this->redrawable, layer, this->tmpStroke, recognized);
			undo->addUndoAction(recognizerUndo);
			layer->addElement(result->getRecognized());

			Range range(recognized->getX(), recognized->getY());
			range.addPoint(recognized->getX() + recognized->getElementWidth(), recognized->getY() + recognized->getElementHeight());

			range.addPoint(this->tmpStroke->getX(), this->tmpStroke->getY());
			range.addPoint(this->tmpStroke->getX() + this->tmpStroke->getElementWidth(), this->tmpStroke->getY() + this->tmpStroke->getElementHeight());

			ListIterator<Stroke *> l = result->getSources();
			while (l.hasNext()) {
				Stroke * s = l.next();

				layer->removeElement(s, false);

				recognizerUndo->addSourceElement(s);

				range.addPoint(s->getX(), s->getY());
				range.addPoint(s->getX() + s->getElementWidth(), s->getY() + s->getElementHeight());
			}

			this->redrawable->rerenderRange(range);

			// delete the result object, this is not needed anymore, the stroke are not deleted with this
			delete result;
		} else {
			layer->addElement(this->tmpStroke);
			this->redrawable->rerenderElement(this->tmpStroke);
		}

	} else {
		layer->addElement(this->tmpStroke);
		this->redrawable->rerenderElement(this->tmpStroke);
	}

	this->tmpStroke = NULL;

	if (currentInputDevice == event->device) {
		currentInputDevice = NULL;
	}

	this->tmpStrokeDrawElem = 0;
}

bool InputHandler::onMotionNotifyEvent(GdkEventMotion * event) {
	XOJ_CHECK_TYPE(InputHandler);

	if (this->tmpStroke != NULL && this->currentInputDevice == event->device) {
		this->addPointToTmpStroke(event);
		return true;
	}
	return false;
}

void InputHandler::startStroke(GdkEventButton * event, StrokeTool tool, double x, double y) {
	XOJ_CHECK_TYPE(InputHandler);

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
	XOJ_CHECK_TYPE(InputHandler);

	return tmpStroke;
}

void InputHandler::resetShapeRecognizer() {
	XOJ_CHECK_TYPE(InputHandler);

	if (this->reco) {
		delete this->reco;
		this->reco = NULL;
	}
}
