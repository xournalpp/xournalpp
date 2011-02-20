#include "EditSelection.h"
#include "Selection.h"
#include "../../gui/XournalWidget.h"
#include <math.h>
#include "../../undo/UndoRedoHandler.h"
#include "../../undo/ScaleUndoAction.h"
#include "../../undo/ColorUndoAction.h"
#include "../../undo/SizeUndoAction.h"
#include "../../undo/FontUndoAction.h"

#include <config.h>
#include <glib/gi18n-lib.h>

EditSelection::EditSelection(UndoRedoHandler * undo, double x, double y, double width, double height, XojPage * page,
		Redrawable * view) {
	this->x = x;
	this->y = y;
	this->width = width;
	this->height = height;

	this->originalWidth = width;
	this->originalHeight = height;

	this->page = page;
	this->layer = this->page->getSelectedLayer();
	this->view = view;
	this->inputView = view;
	this->undo = undo;

	initAttributes();
}

EditSelection::EditSelection(UndoRedoHandler * undo, Selection * selection, Redrawable * view) {
	selection->getSelectedRect(this->x, this->y, this->width, this->height);
	this->page = selection->page;
	this->layer = this->page->getSelectedLayer();
	this->view = view;
	this->inputView = view;

	this->originalWidth = this->width;
	this->originalHeight = this->height;

	this->undo = undo;

	initAttributes();

	for (GList * l = selection->selectedElements; l != NULL; l = l->next) {
		addElementInt((Element *) l->data);
	}

	this->view->deleteViewBuffer();
}

EditSelection::EditSelection(UndoRedoHandler * undo, Element * e, Redrawable * view, XojPage * page) {
	this->page = page;
	this->layer = this->page->getSelectedLayer();
	this->view = view;
	this->inputView = view;
	this->undo = undo;

	this->x = e->getX();
	this->y = e->getY();
	this->width = e->getElementWidth();
	this->height = e->getElementHeight();
	this->originalWidth = this->width;
	this->originalHeight = this->height;

	initAttributes();

	addElementInt(e);

	this->view->deleteViewBuffer();
}

void EditSelection::initAttributes() {
	this->selected = NULL;

	this->relativeX = this->x;
	this->relativeY = this->y;
	this->mouseX = 0;
	this->mouseY = 0;
	this->offsetX = 0;
	this->offsetY = 0;
	this->aspectRatio = false;

	this->documentView = new DocumentView();

	this->selType = CURSOR_SELECTION_NONE;
	this->selX = 0;
	this->selY = 0;

	this->moveUndoAction = NULL;
	this->crBuffer = NULL;
	this->rescaleId = 0;
}

EditSelection::~EditSelection() {
	if(this->rescaleId) {
		g_source_remove(this->rescaleId);
		this->rescaleId = 0;
	}

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

	for (GList * l = this->selected; l != NULL; l = l->next) {
		Element * e = (Element *) l->data;
		e->move(this->x - this->relativeX, this->y - this->relativeY);
		if (scale) {
			e->scale(this->x, this->y, fx, fy);
		}
		this->layer->addElement(e);
	}

	if (scale) {
		ScaleUndoAction * scaleUndo = new ScaleUndoAction(this->page, this->view, this->selected, this->x, this->y, fx,
				fy);
		this->undo->addUndoAction(scaleUndo);
	}

	view->repaint(x - this->offsetX, y - this->offsetY, x + width, y + height);
	g_list_free(this->selected);

	delete this->documentView;

	deleteViewBuffer();
}

void EditSelection::deleteViewBuffer() {
	if (this->crBuffer) {
		cairo_surface_destroy(this->crBuffer);
		this->crBuffer = NULL;
	}
}

void EditSelection::addElementInt(Element * e) {
	layer->removeElement(e, false);
	addElement(e);
}

void EditSelection::addElement(Element * e) {
	this->selected = g_list_append(this->selected, e);

	if (e->rescaleOnlyAspectRatio()) {
		this->aspectRatio = true;
	}
}

void EditSelection::finalizeEditing() {
	this->selX = 0;
	this->selY = 0;
	this->mouseX = 0;
	this->mouseY = 0;
	this->x -= this->offsetX;
	this->y -= this->offsetY;
	this->offsetX = 0;
	this->offsetY = 0;
	this->layer = this->page->getSelectedLayer();

	this->selType = CURSOR_SELECTION_NONE;

	this->inputView = this->view;

	if (this->moveUndoAction) {
		this->moveUndoAction->finalize(this);
		undo->addUndoAction(this->moveUndoAction);
		this->moveUndoAction = NULL;
	}
}

UndoAction * EditSelection::setColor(int color) {
	ColorUndoAction * undo = new ColorUndoAction(this->page, this->layer, this->view);

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
		this->view->redrawDocumentRegion(x1 - this->offsetX, y1 - this->offsetY, x2, y2);

		return undo;
	} else {
		delete undo;
		return NULL;
	}
}

UndoAction * EditSelection::setSize(ToolSize size, const double * thiknessPen, const double * thiknessHilighter,
		const double * thiknessEraser) {

	SizeUndoAction * undo = new SizeUndoAction(this->page, this->layer, this->view);

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
		this->view->redrawDocumentRegion(x1 - this->offsetX, y1 - this->offsetY, x2, y2);

		return undo;
	} else {
		delete undo;
		return NULL;
	}
}

GList * EditSelection::getElements() {
	return this->selected;
}

void EditSelection::setEditMode(CursorSelectionType selType, double x, double y) {
	this->selType = selType;
	this->selX = x;
	this->selY = y;
	this->mouseX = this->selX - this->x;
	this->mouseY = this->selY - this->y;
	this->offsetX = 0;
	this->offsetY = 0;
}

/**
 * TODO LOW PRIO: it should not be possible to move a object out of a page, but at the moment its possible
 * I know it's the same on the original xournal, but it's not user friendly if an object can be "lost"...
 */
void EditSelection::doMove(double dx, double dy, Redrawable * view, XournalWidget * xournal) {
	if (this->moveUndoAction == NULL) {
		this->moveUndoAction = new MoveUndoAction(this->page, this);
	}

	double x1 = this->x;
	double x2 = this->x + this->width;
	double y1 = this->y;
	double y2 = this->y + this->height;

	GtkAllocation alloc = { 0 };
	gtk_widget_get_allocation(inputView->getWidget(), &alloc);

	this->x += dx;
	this->y += dy;

	double zoom = xournal->getZoom();

	// Test if the selection is moved to another page
	int xPos = (this->x + this->mouseX) * zoom + alloc.x;
	int yPos = (this->y + this->mouseY) * zoom + alloc.y;

	PageView * v = xournal->getViewAt(xPos, yPos);
	Redrawable * lastView = NULL;
	if (v != NULL && this->view != v) {
		lastView = this->view;
		this->view = v;

		GtkAllocation alloc1 = { 0 };
		gtk_widget_get_allocation(lastView->getWidget(), &alloc1);
		GtkAllocation alloc2 = { 0 };
		gtk_widget_get_allocation(this->view->getWidget(), &alloc2);

		double zoom = xournal->getZoom();

		double xOffset = alloc2.x - alloc1.x;
		double yOffset = alloc2.y - alloc1.y;

		if (yOffset > 1 || yOffset < -1) {
			if (yOffset > 1) {
				yOffset += XOURNAL_PADDING;
			} else {
				yOffset -= XOURNAL_PADDING;
			}

			yOffset *= zoom;
		}

		if (xOffset > 1 || xOffset < -1) {
			if (xOffset > 1) {
				xOffset += XOURNAL_PADDING;
			} else {
				xOffset -= XOURNAL_PADDING;
			}

			xOffset *= zoom;
		}

		this->offsetX += xOffset;
		this->offsetY += yOffset;

		page = v->getPage();
		this->layer = page->getSelectedLayer();
	}

	if (lastView) {
		//		lastView->redrawDocumentRegion(x1, y1, x2, y2);
		lastView->redraw();

		//		x1 = this->x;
		//		y1 = this->y;
		//
		//		x2 = this->x + this->width;
		//		y2 = this->y + this->height;
	} else {
		//		x1 = MIN(x1, this->x);
		//		y1 = MIN(y1, this->y);
		//
		//		x2 = MAX(x2, this->x + this->width);
		//		y2 = MAX(y2, this->y + this->height);
	}
	//this->view->redrawDocumentRegion(x1 - this->offsetX, y1 - this->offsetY, x2, y2);
	//this->view->redrawDocumentRegion(x1, y1, x2, y2);

	this->view->redraw();
}

void EditSelection::move(double x, double y, Redrawable * view, XournalWidget * xournal) {
	if (this->selType == CURSOR_SELECTION_MOVE) {
		double dx = x - this->selX;
		double dy = y - this->selY;
		this->selX = x;
		this->selY = y;

		doMove(dx, dy, view, xournal);
	} else if (this->selType == CURSOR_SELECTION_TOP_LEFT) {
		double dx = x - this->x;
		double dy = y - this->y;
		double f;
		if (ABS(dy) < ABS(dx)) {
			f = (this->height + dy) / this->height;
		} else {
			f = (this->width + dx) / this->width;
		}

		double oldW = this->width;
		double oldH = this->height;
		this->width /= f;
		this->height /= f;

		this->x += oldW - this->width;
		this->y += oldH - this->height;

		gtk_widget_queue_draw(this->view->getWidget());
	} else if (this->selType == CURSOR_SELECTION_TOP_RIGHT) {
		double dx = x - this->x - this->width;
		double dy = y - this->y;
		double f;
		if (ABS(dy) < ABS(dx)) {
			f = this->height / (this->height + dy);
		} else {
			f = (this->width + dx) / this->width;
		}

		double oldH = this->height;
		this->width *= f;
		this->height *= f;

		this->y += oldH - this->height;

		gtk_widget_queue_draw(this->view->getWidget());
	} else if (this->selType == CURSOR_SELECTION_BOTTOM_LEFT) {
		double dx = x - this->x;
		double dy = y - this->y - this->height;
		double f;
		if (ABS(dy) < ABS(dx)) {
			f = (this->height + dy) / this->height;
		} else {
			f = this->width / (this->width + dx);
		}

		double oldW = this->width;
		this->width *= f;
		this->height *= f;

		this->x += oldW - this->width;

		gtk_widget_queue_draw(this->view->getWidget());
	} else if (this->selType == CURSOR_SELECTION_BOTTOM_RIGHT) {
		double dx = x - this->x - this->width;
		double dy = y - this->y - this->height;
		double f;
		if (ABS(dy) < ABS(dx)) {
			f = (this->height + dy) / this->height;
		} else {
			f = (this->width + dx) / this->width;
		}

		this->width *= f;
		this->height *= f;

		gtk_widget_queue_draw(this->view->getWidget());
	} else if (this->selType == CURSOR_SELECTION_TOP) {
		double dy = y - this->y;
		this->height -= dy;
		this->y += dy;
		gtk_widget_queue_draw(this->view->getWidget());
	} else if (this->selType == CURSOR_SELECTION_BOTTOM) {
		double dy = y - this->y - this->height;
		this->height += dy;
		gtk_widget_queue_draw(this->view->getWidget());
	} else if (this->selType == CURSOR_SELECTION_LEFT) {
		double dx = x - this->x;
		this->width -= dx;
		this->x += dx;

		gtk_widget_queue_draw(this->view->getWidget());
	} else if (this->selType == CURSOR_SELECTION_RIGHT) {
		double dx = x - this->x - this->width;
		this->width += dx;
		gtk_widget_queue_draw(this->view->getWidget());
	}
}

CursorSelectionType EditSelection::getEditMode() {
	return this->selType;
}

void EditSelection::drawAnchorRect(cairo_t * cr, double x, double y, double zoom) {
	GdkColor selectionColor = view->getSelectionColor();
	cairo_set_source_rgb(cr, selectionColor.red / 65536.0, selectionColor.green / 65536.0, selectionColor.blue
			/ 65536.0);
	cairo_rectangle(cr, x - 4 / zoom, y - 4 / zoom, 8 / zoom, 8 / zoom);
	cairo_stroke_preserve(cr);
	cairo_set_source_rgb(cr, 1, 1, 1);
	cairo_fill(cr);
}

CursorSelectionType EditSelection::getSelectionTypeForPos(double x, double y, double zoom) {
	double x1 = this->x * zoom;
	double x2 = (this->x + this->width) * zoom;
	double y1 = this->y * zoom;
	double y2 = (this->y + this->height) * zoom;

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

bool EditSelection::repaintSelection(EditSelection * selection) {
	selection->deleteViewBuffer();
	gtk_widget_queue_draw(selection->view->getWidget());
	selection->rescaleId = 0;
	return false;
}

void EditSelection::paint(cairo_t * cr, GdkEventExpose *event, double zoom) {
	double x = this->x - this->offsetX;
	double y = this->y - this->offsetY;

	double fx = this->width / this->originalWidth;
	double fy = this->height / this->originalHeight;

	if (this->crBuffer == NULL) {
		this->crBuffer = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, this->width * zoom, this->height * zoom);
		cairo_t * cr2 = cairo_create(this->crBuffer);
		//		cairo_set_source_rgb(cr2, 1, 0, 0);
		//		cairo_rectangle(cr2, 0, 0, 100, 100);
		//		cairo_fill(cr2);

		cairo_scale(cr2, zoom * fx, zoom * fy);
		cairo_translate(cr2, -this->relativeX, -this->relativeY);
		this->documentView->drawSelection(cr2, this);

		cairo_destroy(cr2);
	}

	cairo_save(cr);

	if ((int) (this->width * zoom) != (int) cairo_image_surface_get_width(this->crBuffer)
			|| (int) (this->height * zoom) != (int) cairo_image_surface_get_height(this->crBuffer)) {
		if (!this->rescaleId) {
			this->rescaleId = g_idle_add((GSourceFunc) repaintSelection, this);
		}
	}

	cairo_scale(cr, 1 / zoom, 1 / zoom);

	cairo_set_source_surface(cr, this->crBuffer, x * zoom, y * zoom);
	cairo_paint(cr);

	cairo_restore(cr);

	cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

	GdkColor selectionColor = view->getSelectionColor();

	// set the line always the same size on display
	cairo_set_line_width(cr, 1 / zoom);

	const double dashes[] = { 10.0 / zoom, 10.0 / zoom };
	cairo_set_dash(cr, dashes, sizeof(dashes) / sizeof(dashes[0]), 0);
	cairo_set_source_rgb(cr, selectionColor.red / 65536.0, selectionColor.green / 65536.0, selectionColor.blue
			/ 65536.0);

	cairo_rectangle(cr, x, y, width, height);

	cairo_stroke_preserve(cr);
	cairo_set_source_rgba(cr, selectionColor.red / 65536.0, selectionColor.green / 65536.0, selectionColor.blue
			/ 65536.0, 0.3);
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

Redrawable * EditSelection::getInputView() {
	return inputView;
}

Redrawable * EditSelection::getView() {
	return this->view;
}

XojPage * EditSelection::getPage() {
	return this->page;
}

UndoAction * EditSelection::setFont(XojFont & font) {
	double x1 = 0.0 / 0.0;
	double x2 = 0.0 / 0.0;
	double y1 = 0.0 / 0.0;
	double y2 = 0.0 / 0.0;

	FontUndoAction * undo = new FontUndoAction(this->page, this->layer, this->view);

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
		this->view->redrawDocumentRegion(x1, y1, x2, y2);
		return undo;
	}
	delete undo;
	return NULL;
}

void EditSelection::fillUndoItemAndDelete(DeleteUndoAction * undo) {
	Layer * layer = this->page->getSelectedLayer();
	for (GList * l = this->selected; l != NULL; l = l->next) {
		Element * e = (Element *) l->data;
		undo->addElement(layer, e, layer->indexOf(e));
		layer->removeElement(e, false);
	}
}

double EditSelection::getX() {
	return this->x;
}

double EditSelection::getY() {
	return this->y;
}

double EditSelection::getWidth() {
	return this->width;
}

double EditSelection::getHeight() {
	return this->height;
}

