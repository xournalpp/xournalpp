#include "EditSelection.h"
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

#include <math.h>

EditSelection::EditSelection(UndoRedoHandler * undo, double x, double y, double width, double height, XojPage * page, PageView * view) {
	XOJ_INIT_TYPE(EditSelection);

	this->x = x;
	this->y = y;
	this->width = width;
	this->height = height;

	contstruct(undo, view, page);
}

EditSelection::EditSelection(UndoRedoHandler * undo, Selection * selection, PageView * view) {
	XOJ_INIT_TYPE(EditSelection);

	selection->getSelectedRect(this->x, this->y, this->width, this->height);

	contstruct(undo, view, view->getPage());

	for (GList * l = selection->selectedElements; l != NULL; l = l->next) {
		Element * e = (Element *) l->data;
		this->sourceLayer->removeElement(e, false);
		addElement(e);
	}

	view->rerenderPage();
}

EditSelection::EditSelection(UndoRedoHandler * undo, Element * e, PageView * view, XojPage * page) {
	XOJ_INIT_TYPE(EditSelection);

	this->x = e->getX();
	this->y = e->getY();
	this->width = e->getElementWidth();
	this->height = e->getElementHeight();

	contstruct(undo, view, page);
}

/**
 * Our internal constructor
 */
void EditSelection::contstruct(UndoRedoHandler * undo, PageView * view, XojPage * sourcePage) {
	XOJ_CHECK_TYPE(EditSelection);

	this->view = view;
	this->undo = undo;
	this->sourcePage = sourcePage;
	this->sourceLayer = this->sourcePage->getSelectedLayer();

	this->aspectRatio = false;
	this->selected = NULL;

	this->offsetX = 0;
	this->offsetY = 0;
	this->originalWidth = this->width;
	this->originalHeight = this->height;
	this->relativeX = ceil(this->x);
	this->relativeY = ceil(this->y);

	this->crBuffer = NULL;

	this->rescaleId = 0;
}

EditSelection::~EditSelection() {
	XOJ_CHECK_TYPE(EditSelection);

	if (this->rescaleId) {
		g_source_remove(this->rescaleId);
		this->rescaleId = 0;
	}

	finalizeSelection();

	this->view = NULL;
	this->undo = NULL;

	this->sourcePage = NULL;
	this->sourceLayer = NULL;

	deleteViewBuffer();

	g_list_free(this->selected);
	this->selected = NULL;

	deleteViewBuffer();

	XOJ_RELEASE_TYPE(EditSelection);
}

/**
 * Finishes all pending changes, move the elements, scale the elements and add
 * them to new layer if any or to the old if no new layer
 */
void EditSelection::finalizeSelection() {
	XOJ_CHECK_TYPE(EditSelection);

	double fx = this->width / this->originalWidth;
	double fy = this->height / this->originalHeight;

	bool scale = false;
	if (this->aspectRatio) {
		double f = (fx + fy) / 2;
		fx = f;
		fy = f;
	}
	if (this->width != this->originalWidth || this->height != this->originalHeight) {
		scale = true;
	}

	Layer * layer = this->sourceLayer;
	// TODO: add to new layer

	for (GList * l = this->selected; l != NULL; l = l->next) {
		Element * e = (Element *) l->data;
		e->move(this->x - this->relativeX, this->y - this->relativeY);
		if (scale) {
			e->scale(this->x, this->y, fx, fy);
		}
		layer->addElement(e);
	}

	if (scale) {
		// TODO: ????????????????????????
		//		ScaleUndoAction * scaleUndo = new ScaleUndoAction(this->page, this->view, this->selected, this->x, this->y, fx, fy);
		//		this->undo->addUndoAction(scaleUndo);
	}

	this->view->getXournal()->repaintSelection();
}

/**
 * get the X cooridnate relative to the provided view (getView())
 */
double EditSelection::getXOnView() {
	XOJ_CHECK_TYPE_RET(EditSelection, 0);

	return this->x - this->offsetX;
}

/**
 * get the Y cooridnate relative to the provided view (getView())
 */
double EditSelection::getYOnView() {
	XOJ_CHECK_TYPE_RET(EditSelection, 0);

	return this->y - this->offsetY;
}

/**
 * get the width in document coordinates (multiple with zoom)
 */
double EditSelection::getWidth() {
	XOJ_CHECK_TYPE_RET(EditSelection, 0);

	return this->width;
}

/**
 * get the height in document coordinates (multiple with zoom)
 */
double EditSelection::getHeight() {
	XOJ_CHECK_TYPE_RET(EditSelection, 0);

	return this->height;
}

/**
 * get the source page (where the selection was done)
 */
XojPage * EditSelection::getSourcePage() {
	XOJ_CHECK_TYPE_RET(EditSelection, NULL);

	return this->sourcePage;
}

/**
 * get the target page if not the same as the source page, if the selection is moved to a new page
 */
XojPage * EditSelection::getTargetPage() {
	XOJ_CHECK_TYPE_RET(EditSelection, NULL);

	return NULL;
}

/**
 * Sets the tool size for pen or eraser, returs an undo action
 * (or NULL if nothing is done)
 */
UndoAction * EditSelection::setSize(ToolSize size, const double * thiknessPen, const double * thiknessHilighter, const double * thiknessEraser) {
	XOJ_CHECK_TYPE_RET(EditSelection, NULL);

	SizeUndoAction * undo = new SizeUndoAction(this->sourcePage, this->sourceLayer, this->view);

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
		double x1 = this->x;
		double x2 = this->x + this->width;
		double y1 = this->y;
		double y2 = this->y + this->height;

		this->deleteViewBuffer();
		this->view->getXournal()->repaintSelection();

		return undo;
	} else {
		delete undo;
		return NULL;
	}
}

/**
 * Set the color of all elements, return an undo action
 * (Or NULL if nothing done, e.g. because there is only an image)
 */
UndoAction * EditSelection::setColor(int color) {
	XOJ_CHECK_TYPE_RET(EditSelection, NULL);

	ColorUndoAction * undo = new ColorUndoAction(this->sourcePage, this->sourceLayer, this->view);

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
		double x1 = this->x;
		double x2 = this->x + this->width;
		double y1 = this->y;
		double y2 = this->y + this->height;

		this->deleteViewBuffer();
		this->view->getXournal()->repaintSelection();

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
UndoAction * EditSelection::setFont(XojFont & font) {
	XOJ_CHECK_TYPE_RET(EditSelection, NULL);

	double x1 = 0.0 / 0.0;
	double x2 = 0.0 / 0.0;
	double y1 = 0.0 / 0.0;
	double y2 = 0.0 / 0.0;

	FontUndoAction * undo = new FontUndoAction(this->sourcePage, this->sourceLayer, this->view);

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
		this->view->getXournal()->repaintSelection();
		return undo;
	}
	delete undo;
	return NULL;
}

/**
 * Add an element to the this selection
 */
void EditSelection::addElement(Element * e) {
	XOJ_CHECK_TYPE(EditSelection);

	this->selected = g_list_append(this->selected, e);

	if (e->rescaleOnlyAspectRatio()) {
		this->aspectRatio = true;
	}
}

/**
 * Returns all containig elements of this selections
 */
ListIterator<Element *> EditSelection::getElements() {
	XOJ_CHECK_TYPE_RET(EditSelection, NULL);

	return ListIterator<Element *> (this->selected);
}

/**
 * Finish the current movement
 * (should be called in the mouse-button-released event handler)
 */
void EditSelection::finalizeEditing() {
	XOJ_CHECK_TYPE(EditSelection);

	// TODO ????????????????????

}

/**
 * Move the selection
 */
void EditSelection::moveSelection(double dx, double dy) {
	XOJ_CHECK_TYPE(EditSelection);

	this->offsetX -= dx;
	this->offsetY -= dy;

	ensureWithinVisibleArea();

	int x = this->view->getX() - this->offsetX + this->relativeX;
	int y = this->view->getY() - this->offsetY + this->relativeY;
	double zoom = this->view->getXournal()->getZoom();
	this->view->getXournal()->ensureRectIsVisible(x, y, this->width * zoom, this->height * zoom);

	this->view->getXournal()->repaintSelection();
}

/**
 * If the selection is outside the visible area correct the coordinates
 */
void EditSelection::ensureWithinVisibleArea() {
	XOJ_CHECK_TYPE(EditSelection);

	//TODO: scroll to this point if not in visible area

	double zoom = this->view->getXournal()->getZoom();
	int x = this->view->getX() - this->offsetX + this->relativeX;
	if (x < 0) {
		this->offsetX += x;
	}
	int maxX = this->view->getXournal()->getMaxAreaX();
	if (maxX < x + this->width * zoom) {
		this->offsetX += (x + this->width * zoom) - maxX;
	}

	int y = this->view->getY() - this->offsetY + this->relativeY;
	if (y < 0) {
		this->offsetY += y;
	}
	int maxY = this->view->getXournal()->getMaxAreaY();
	if (maxY < y + this->height * zoom) {
		this->offsetY += (y + this->height * zoom) - maxY;
	}
}

/**
 * Get the cursor type for the current position (if 0 then the default cursor should be used)
 */
CursorSelectionType EditSelection::getSelectionTypeForPos(double x, double y, double zoom) {
	XOJ_CHECK_TYPE_RET(EditSelection, CURSOR_SELECTION_NONE);

	double x1 = getXOnView() * zoom;
	double x2 = x1 + (this->width * zoom);
	double y1 = getYOnView() * zoom;
	double y2 = y1 + (this->height * zoom);

	const int EDGE_PADDING = 5;
	const int BORDER_PADDING = 3;

	if (x1 - EDGE_PADDING <= x && x <= x1 + EDGE_PADDING && y1 - EDGE_PADDING <= y && y <= y1 + EDGE_PADDING) {
		return CURSOR_SELECTION_TOP_LEFT;
	}

	if (x2 - EDGE_PADDING <= x && x <= x2 + EDGE_PADDING && y1 - EDGE_PADDING <= y && y <= y1 + EDGE_PADDING) {
		return CURSOR_SELECTION_TOP_RIGHT;
	}

	if (x1 - EDGE_PADDING <= x && x <= x1 + EDGE_PADDING && y2 - EDGE_PADDING <= y && y <= y2 + EDGE_PADDING) {
		return CURSOR_SELECTION_BOTTOM_LEFT;
	}

	if (x2 - EDGE_PADDING <= x && x <= x2 + EDGE_PADDING && y2 - EDGE_PADDING <= y && y <= y2 + EDGE_PADDING) {
		return CURSOR_SELECTION_BOTTOM_RIGHT;
	}

	if (!this->aspectRatio) {
		if (x1 <= x && x2 >= x) {
			if (y1 - BORDER_PADDING <= y && y <= y1 + BORDER_PADDING) {
				return CURSOR_SELECTION_TOP;
			}

			if (y2 - BORDER_PADDING <= y && y <= y2 + BORDER_PADDING) {
				return CURSOR_SELECTION_BOTTOM;
			}
		}

		if (y1 <= y && y2 >= y) {
			if (x1 - BORDER_PADDING <= x && x <= x1 + BORDER_PADDING) {
				return CURSOR_SELECTION_LEFT;
			}

			if (x2 - BORDER_PADDING <= x && x <= x2 + BORDER_PADDING) {
				return CURSOR_SELECTION_RIGHT;
			}
		}
	}

	if (x1 <= x && x <= x2 && y1 <= y && y <= y2) {
		return CURSOR_SELECTION_MOVE;
	}

	return CURSOR_SELECTION_NONE;
}

/**
 * Paints the selection to cr, with the given zoom factor. The coordinates of cr
 * should be relative to the provideded view by getView() (use translateEvent())
 */
void EditSelection::paint(cairo_t * cr, double zoom) {
	XOJ_CHECK_TYPE(EditSelection);

	double x = this->x - this->offsetX;
	double y = this->y - this->offsetY;

	double fx = this->width / this->originalWidth;
	double fy = this->height / this->originalHeight;

	if (this->crBuffer == NULL) {
		this->crBuffer = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, this->width * zoom, this->height * zoom);
		cairo_t * cr2 = cairo_create(this->crBuffer);

		cairo_scale(cr2, zoom * fx, zoom * fy);
		cairo_translate(cr2, -this->relativeX, -this->relativeY);
		DocumentView view;
		view.drawSelection(cr2, this);

		cairo_destroy(cr2);
	}

	cairo_save(cr);

	if ((int) (this->width * zoom) != (int) cairo_image_surface_get_width(this->crBuffer) || (int) (this->height * zoom)
			!= (int) cairo_image_surface_get_height(this->crBuffer)) {
		if (!this->rescaleId) {
			this->rescaleId = g_idle_add((GSourceFunc) repaintSelection, this);
		}
	}

	cairo_scale(cr, 1 / zoom, 1 / zoom);

	cairo_set_source_surface(cr, this->crBuffer, (int) (x * zoom), (int) (y * zoom));
	cairo_paint(cr);

	cairo_restore(cr);

	cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

	GdkColor selectionColor = view->getSelectionColor();

	// set the line always the same size on display
	cairo_set_line_width(cr, 1 / zoom);

	const double dashes[] = { 10.0 / zoom, 10.0 / zoom };
	cairo_set_dash(cr, dashes, sizeof(dashes) / sizeof(dashes[0]), 0);
	cairo_set_source_rgb(cr, selectionColor.red / 65536.0, selectionColor.green / 65536.0, selectionColor.blue / 65536.0);

	cairo_rectangle(cr, x, y, width, height);

	cairo_stroke_preserve(cr);
	cairo_set_source_rgba(cr, selectionColor.red / 65536.0, selectionColor.green / 65536.0, selectionColor.blue / 65536.0, 0.3);
	cairo_fill(cr);

	cairo_set_dash(cr, NULL, 0, 0);

	if (!this->aspectRatio) {
		// top
		drawAnchorRect(cr, x + width / 2, y, zoom);
		// bottom
		drawAnchorRect(cr, x + width / 2, y + height, zoom);
		// left
		drawAnchorRect(cr, x, y + height / 2, zoom);
		// right
		drawAnchorRect(cr, x + width, y + height / 2, zoom);
	}

	// top left
	drawAnchorRect(cr, x, y, zoom);
	// top right
	drawAnchorRect(cr, x + width, y, zoom);
	// bottom left
	drawAnchorRect(cr, x, y + height, zoom);
	// bottom right
	drawAnchorRect(cr, x + width, y + height, zoom);
}

/**
 * Callback to redrawing the buffer asynchron
 */
bool EditSelection::repaintSelection(EditSelection * selection) {
	XOJ_CHECK_TYPE_OBJ_RET(selection, EditSelection, false); //TODO: return true or false for no recall

	gdk_threads_enter();

	// delete the selection buffer, force a redraw
	selection->deleteViewBuffer();
	selection->view->repaintRect(selection->x, selection->y, selection->width, selection->height);
	selection->rescaleId = 0;

	gdk_threads_leave();
	return false;
}

/**
 * draws an idicator where you can scale the selection
 */
void EditSelection::drawAnchorRect(cairo_t * cr, double x, double y, double zoom) {
	XOJ_CHECK_TYPE(EditSelection);

	GdkColor selectionColor = view->getSelectionColor();
	cairo_set_source_rgb(cr, selectionColor.red / 65536.0, selectionColor.green / 65536.0, selectionColor.blue / 65536.0);
	cairo_rectangle(cr, x - 4 / zoom, y - 4 / zoom, 8 / zoom, 8 / zoom);
	cairo_stroke_preserve(cr);
	cairo_set_source_rgb(cr, 1, 1, 1);
	cairo_fill(cr);
}

PageView * EditSelection::getView() {
	XOJ_CHECK_TYPE_RET(EditSelection, NULL);

	return this->view;
}

/**
 * Delete our internal View buffer,
 * it will be recreated when the selection is painted next time
 */
void EditSelection::deleteViewBuffer() {
	XOJ_CHECK_TYPE(EditSelection);

	if (this->crBuffer) {
		cairo_surface_destroy(this->crBuffer);
		this->crBuffer = NULL;
	}
}

//EditSelection::~EditSelection() {
//	if (this->rescaleId) {
//		g_source_remove(this->rescaleId);
//		this->rescaleId = 0;
//	}
//
//	double fx = this->width / this->originalWidth;
//	double fy = this->height / this->originalHeight;
//
//	bool scale = false;
//	if (this->aspectRatio) {
//		double f = (fx + fy) / 2;
//		fx = f;
//		fy = f;
//	}
//	if (this->width != this->originalWidth || this->height != this->originalHeight) {
//		scale = true;
//	}
//
//	for (GList * l = this->selected; l != NULL; l = l->next) {
//		Element * e = (Element *) l->data;
//		e->move(this->x - this->relativeX, this->y - this->relativeY);
//		if (scale) {
//			e->scale(this->x, this->y, fx, fy);
//		}
//		this->layer->addElement(e);
//	}
//
//	if (scale) {
//		ScaleUndoAction * scaleUndo = new ScaleUndoAction(this->page, this->view, this->selected, this->x, this->y, fx, fy);
//		this->undo->addUndoAction(scaleUndo);
//	}
//
//	view->rerenderRect(x - this->offsetX, y - this->offsetY, width, height);
//	g_list_free(this->selected);
//
//	delete this->documentView;
//
//	deleteViewBuffer();
//}

