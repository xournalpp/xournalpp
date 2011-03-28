#include "EditSelectionContents.h"
#include "../../undo/UndoRedoHandler.h"
#include "../../model/Element.h"
#include "../../model/Stroke.h"
#include "../../model/Text.h"
#include "Selection.h"
#include "../../gui/PageView.h"
#include "../../view/DocumentView.h"
#include "../../undo/SizeUndoAction.h"
#include "../../undo/ColorUndoAction.h"
#include "../../undo/FontUndoAction.h"
#include "../../gui/XournalView.h"
#include "../../gui/pageposition/PagePositionHandler.h"
#include "../../control/Control.h"
#include "../../model/Document.h"

#include <math.h>

EditSelectionContents::EditSelectionContents(double x, double y, double width, double height, XojPage * sourcePage, Layer * sourceLayer, PageView * sourceView) {
	XOJ_INIT_TYPE(EditSelectionContents);

	this->selected = NULL;
	this->crBuffer = NULL;

	this->rescaleId = 0;

	this->originalWidth = width;
	this->originalHeight = height;
	this->relativeX = ceil(x);
	this->relativeY = ceil(y);

	this->sourcePage = sourcePage;
	this->sourceLayer = sourceLayer;
	this->sourceView = sourceView;
}

EditSelectionContents::~EditSelectionContents() {
	XOJ_CHECK_TYPE(EditSelectionContents);

	if (this->rescaleId) {
		g_source_remove(this->rescaleId);
		this->rescaleId = 0;
	}

	g_list_free(this->selected);
	this->selected = NULL;

	deleteViewBuffer();

	XOJ_RELEASE_TYPE(EditSelectionContents);
}

/**
 * Add an element to the this selection
 */
void EditSelectionContents::addElement(Element * e) {
	XOJ_CHECK_TYPE(EditSelectionContents);

	this->selected = g_list_append(this->selected, e);
}

/**
 * Returns all containig elements of this selections
 */
ListIterator<Element *> EditSelectionContents::getElements() {
	XOJ_CHECK_TYPE(EditSelectionContents);

	return ListIterator<Element *> (this->selected);
}

/**
 * Sets the tool size for pen or eraser, returs an undo action
 * (or NULL if nothing is done)
 */
UndoAction * EditSelectionContents::setSize(ToolSize size, const double * thiknessPen, const double * thiknessHilighter, const double * thiknessEraser) {
	XOJ_CHECK_TYPE(EditSelectionContents);

	SizeUndoAction * undo = new SizeUndoAction(this->sourcePage, this->sourceLayer, this->sourceView);

	bool found = false;

	for (GList * l = this->selected; l != NULL; l = l->next) {
		Element * e = (Element *) l->data;
		if (e->getType() == ELEMENT_STROKE) {
			Stroke * s = (Stroke *) e;
			StrokeTool tool = s->getToolType();

			double originalWidth = s->getWidth();

			int pointCount = s->getPointCount();
			double * originalPressure = SizeUndoAction::getPressure(s);

			if (tool == STROKE_TOOL_PEN) {
				s->setWidth(thiknessPen[size]);
			} else if (tool == STROKE_TOOL_HIGHLIGHTER) {
				s->setWidth(thiknessHilighter[size]);
			} else if (tool == STROKE_TOOL_ERASER) {
				s->setWidth(thiknessEraser[size]);
			}

			// scale the stroke
			double factor = s->getWidth() / originalWidth;
			s->scalePressure(factor);

			// save the new pressure
			double * newPressure = SizeUndoAction::getPressure(s);

			undo->addStroke(s, originalWidth, s->getWidth(), originalPressure, newPressure, pointCount);
			found = true;
		}
	}

	if (found) {
		this->deleteViewBuffer();
		this->sourceView->getXournal()->repaintSelection();

		return undo;
	} else {
		delete undo;
		return NULL;
	}
}

/**
 * Sets the font of all containing text elements, return an undo action
 * (or NULL if there are no Text elements)
 */
UndoAction * EditSelectionContents::setFont(XojFont & font) {
	XOJ_CHECK_TYPE(EditSelectionContents);

	double x1 = 0.0 / 0.0;
	double x2 = 0.0 / 0.0;
	double y1 = 0.0 / 0.0;
	double y2 = 0.0 / 0.0;

	FontUndoAction * undo = new FontUndoAction(this->sourcePage, this->sourceLayer, this->sourceView);

	for (GList * l = this->selected; l != NULL; l = l->next) {
		Element * e = (Element *) l->data;
		if (e->getType() == ELEMENT_TEXT) {
			Text * t = (Text *) e;
			undo->addStroke(t, t->getFont(), font);

			if (isnan(x1)) {
				x1 = t->getX();
				y1 = t->getY();
				x2 = t->getX() + t->getElementWidth();
				y2 = t->getY() + t->getElementHeight();
			} else {
				// size with old font
				x1 = MIN(x1, t->getX());
				y1 = MIN(y1, t->getY());

				x2 = MAX(x2, t->getX() + t->getElementWidth());
				y2 = MAX(y2, t->getY() + t->getElementHeight());
			}

			t->setFont(font);

			// size with new font
			x1 = MIN(x1, t->getX());
			y1 = MIN(y1, t->getY());

			x2 = MAX(x2, t->getX() + t->getElementWidth());
			y2 = MAX(y2, t->getY() + t->getElementHeight());
		}
	}

	if (!isnan(x1)) {
		this->deleteViewBuffer();
		this->sourceView->getXournal()->repaintSelection();
		return undo;
	}
	delete undo;
	return NULL;
}

/**
 * Set the color of all elements, return an undo action
 * (Or NULL if nothing done, e.g. because there is only an image)
 */
UndoAction * EditSelectionContents::setColor(int color) {
	XOJ_CHECK_TYPE(EditSelectionContents);

	ColorUndoAction * undo = new ColorUndoAction(this->sourcePage, this->sourceLayer, this->sourceView);

	bool found = false;

	for (GList * l = this->selected; l != NULL; l = l->next) {
		Element * e = (Element *) l->data;
		if (e->getType() == ELEMENT_TEXT || e->getType() == ELEMENT_STROKE) {
			int lastColor = e->getColor();
			e->setColor(color);
			undo->addStroke(e, lastColor, e->getColor());

			found = true;
		}
	}

	if (found) {
		this->deleteViewBuffer();
		this->sourceView->getXournal()->repaintSelection();

		return undo;
	} else {
		delete undo;
		return NULL;
	}

	return NULL;
}

/**
 * Callback to redrawing the buffer asynchron
 */
bool EditSelectionContents::repaintSelection(EditSelectionContents * selection) {
	XOJ_CHECK_TYPE_OBJ(selection, EditSelectionContents);

	gdk_threads_enter();

	// TODO: debug

	//	// delete the selection buffer, force a redraw
	//	selection->deleteViewBuffer();
	//	selection->view->repaintRect(selection->x, selection->y, selection->width, selection->height);
	//	selection->rescaleId = 0;

	gdk_threads_leave();
	return false;
}

/**
 * Delete our internal View buffer,
 * it will be recreated when the selection is painted next time
 */
void EditSelectionContents::deleteViewBuffer() {
	XOJ_CHECK_TYPE(EditSelectionContents);

	if (this->crBuffer) {
		cairo_surface_destroy(this->crBuffer);
		this->crBuffer = NULL;
	}
}

/**
 * Gets the original width of the contents
 */
double EditSelectionContents::getOriginalWidth() {
	return this->originalWidth;
}

/**
 * Gets the original height of the contents
 */
double EditSelectionContents::getOriginalHeight() {
	return this->originalHeight;
}

/**
 * The contents of the selection
 */
void EditSelectionContents::finalizeSelection(double width, double height, bool aspectRatio, Layer * layer) {
	double fx = width / this->originalWidth;
	double fy = height / this->originalHeight;

	bool scale = false;
	if (aspectRatio) {
		double f = (fx + fy) / 2;
		fx = f;
		fy = f;
	}
	if (width != this->originalWidth || height != this->originalHeight) {
		scale = true;
	}

	// TODO: debug
//	for (GList * l = this->selected; l != NULL; l = l->next) {
//		Element * e = (Element *) l->data;
//		e->move(this->x - this->relativeX, this->y - this->relativeY);
//		if (scale) {
//			e->scale(this->x, this->y, fx, fy);
//		}
//		layer->addElement(e);
//	}

	if (scale) {
		// TODO: ????????????????????????
		//		ScaleUndoAction * scaleUndo = new ScaleUndoAction(this->page, this->view, this->selected, this->x, this->y, fx, fy);
		//		this->undo->addUndoAction(scaleUndo);
	}

}

/**
 * paints the selection
 */
void EditSelectionContents::paint(cairo_t * cr, double x, double y, double width, double height, double zoom) {
	double fx = width / this->originalWidth;
	double fy = height / this->originalHeight;

	if (this->crBuffer == NULL) {
		this->crBuffer = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width * zoom, height * zoom);
		cairo_t * cr2 = cairo_create(this->crBuffer);

		cairo_scale(cr2, zoom * fx, zoom * fy);
		cairo_translate(cr2, -this->relativeX, -this->relativeY);
		DocumentView view;
		view.drawSelection(cr2, this);

		cairo_destroy(cr2);
	}

	int w = cairo_image_surface_get_width(this->crBuffer);
	int h = cairo_image_surface_get_height(this->crBuffer);
	if ((int) (width * zoom) != w || (int) (height * zoom) != h) {
		if (!this->rescaleId) {
			this->rescaleId = g_idle_add((GSourceFunc) repaintSelection, this);
		}
	}

	cairo_set_source_surface(cr, this->crBuffer, (int) (x * zoom), (int) (y * zoom));
	cairo_paint(cr);

}
