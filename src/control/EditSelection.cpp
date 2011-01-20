#include "EditSelection.h"
#include "Selection.h"
#include "../gui/XournalWidget.h"
#include "../gettext.h"
#include <math.h>

EditSelection::EditSelection(double x, double y, double width, double height, XojPage * page, Redrawable * view) {
	this->x = x;
	this->y = y;
	this->width = width;
	this->height = height;
	this->page = page;
	this->layer = this->page->getSelectedLayer();
	this->view = view;
	this->inputView = view;

	initAttributes();
}

EditSelection::EditSelection(Selection * selection, Redrawable * view) {
	selection->getSelectedRect(this->x, this->y, this->width, this->height);
	this->page = selection->page;
	this->layer = this->page->getSelectedLayer();
	this->view = view;
	this->inputView = view;

	initAttributes();

	for (GList * l = selection->selectedElements; l != NULL; l = l->next) {
		addElementInt((Element *) l->data);
	}

	this->view->deleteViewBuffer();
}

EditSelection::EditSelection(Element * e, Redrawable * view, XojPage * page) {
	this->page = page;
	this->layer = this->page->getSelectedLayer();
	this->view = view;
	this->inputView = view;

	this->x = e->getX();
	this->y = e->getY();
	this->width = e->getElementWidth();
	this->height = e->getElementHeight();

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

	this->documentView = new DocumentView();

	this->selType = CURSOR_SELECTION_NONE;
	this->selX = 0;
	this->selY = 0;

	this->undo = NULL;
	this->lastUndoAction = NULL;
}

EditSelection::~EditSelection() {
	for (GList * l = this->selected; l != NULL; l = l->next) {
		Element * e = (Element *) l->data;
		e->move(-this->relativeX + this->x, -this->relativeY + this->y);
		e->finalizeMove();
		this->layer->addElement(e);
	}

	view->deleteViewBuffer();
	view->redrawDocumentRegion(x - this->offsetX, y - this->offsetY, x + width, y + height);
	g_list_free(this->selected);

	delete this->documentView;
}

void EditSelection::addElementInt(Element * e) {
	layer->removeElement(e, false);
	this->selected = g_list_append(this->selected, e);
}

void EditSelection::addElement(Element * e) {
	this->selected = g_list_append(this->selected, e);
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

	if (this->undo) {
		this->undo->finalize(this);
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

			if (tool == STROKE_TOOL_PEN) {
				s->setWidth(thiknessPen[size]);
			} else if (tool == STROKE_TOOL_HIGHLIGHTER) {
				s->setWidth(thiknessHilighter[size]);
			} else if (tool == STROKE_TOOL_ERASER) {
				s->setWidth(thiknessEraser[size]);
			}

			found = true;
			undo->addStroke(s, originalWidth, s->getWidth());
		}
	}

	if (found) {
		double x1 = this->x;
		double x2 = this->x + this->width;
		double y1 = this->y;
		double y2 = this->y + this->height;

		this->view->redrawDocumentRegion(x1 - this->offsetX, y1 - this->offsetY, x2, y2);

		return undo;
	} else {
		delete undo;
		return NULL;
	}
}

MoveUndoAction * EditSelection::getUndoAction() {
	this->lastUndoAction = this->undo;
	this->undo = NULL;
	return this->lastUndoAction;
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
 * TODO: it should not be possible to move a object out of a page, but at the moment its possible
 * I know it's the same on the original xournal, but it's not user friendly if an object can be lost...
 */
void EditSelection::doMove(double dx, double dy, Redrawable * view, XournalWidget * xournal) {
	if (this->undo == NULL) {
		this->undo = new MoveUndoAction(this->page, this);
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

	//	x1 = MIN(x1, this->x);
	//	y1 = MIN(y1, this->y);
	//
	//	x2 = MAX(x2, this->x + this->width);
	//	y2 = MAX(y2, this->y + this->height);

	if (lastView) {
		gtk_widget_queue_draw(lastView->getWidget());
	}
	//this->view->redrawDocumentRegion(x1 - this->offsetX, y1 - this->offsetY, x2, y2);

	gtk_widget_queue_draw(this->view->getWidget());
}

void EditSelection::move(double x, double y, Redrawable * view, XournalWidget * xournal) {
	if (this->selType == CURSOR_SELECTION_MOVE) {
		double dx = x - this->selX;
		double dy = y - this->selY;
		this->selX = x;
		this->selY = y;

		doMove(dx, dy, view, xournal);
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

	if (x1 - 3 <= x && x <= x1 + 3 && y1 - 3 <= y && y <= y1 + 3) {
		return CURSOR_SELECTION_TOP_LEFT;
	}

	if (x2 - 3 <= x && x <= x2 + 3 && y1 - 3 <= y && y <= y1 + 3) {
		return CURSOR_SELECTION_TOP_RIGHT;
	}

	if (x1 - 3 <= x && x <= x1 + 3 && y2 - 3 <= y && y <= y2 + 3) {
		return CURSOR_SELECTION_BOTTOM_LEFT;
	}

	if (x2 - 3 <= x && x <= x2 + 3 && y2 - 3 <= y && y <= y2 + 3) {
		return CURSOR_SELECTION_BOTTOM_RIGHT;
	}

	if (y1 - 2 <= y && y <= y1 + 2) {
		return CURSOR_SELECTION_TOP;
	}

	if (y2 - 2 <= y && y <= y2 + 2) {
		return CURSOR_SELECTION_BOTTOM;
	}

	if (x1 - 2 <= x && x <= x1 + 2) {
		return CURSOR_SELECTION_LEFT;
	}

	if (x2 - 2 <= x && x <= x2 + 2) {
		return CURSOR_SELECTION_RIGHT;
	}

	if (x1 <= x && x <= x2 && y1 <= y && y <= y2) {
		return CURSOR_SELECTION_MOVE;
	}

	return CURSOR_SELECTION_NONE;
}

void EditSelection::paint(cairo_t * cr, GdkEventExpose *event, double zoom) {
	double x = this->x - this->offsetX;
	double y = this->y - this->offsetY;

	cairo_save(cr);
	cairo_translate(cr, -this->relativeX + x, -this->relativeY + y);
	this->documentView->drawSelection(cr, this);
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

//////////////////////////////////////////////////////////////////////

class MoveUndoEntry {
public:
	MoveUndoEntry(Element * e, double x, double y) {
		this->e = e;
		this->x = x;
		this->y = y;
	}

	Element * e;
	double x;
	double y;
};

MoveUndoAction::MoveUndoAction(XojPage * page, EditSelection * selection) {
	this->page = page;
	this->newPage = NULL;
	this->originalPos = NULL;
	this->newPos = NULL;
	this->oldLayer = selection->layer;
	this->newLayer = NULL;
	this->origView = selection->view;
	this->newView = NULL;

	if (selection->lastUndoAction) {
		selection->lastUndoAction->newLayer = selection->layer;
	}

	for (GList * l = selection->selected; l != NULL; l = l->next) {
		Element * e = (Element *) l->data;
		this->originalPos = g_list_append(this->originalPos, new MoveUndoEntry(e, e->getX(), e->getY()));
	}
}

MoveUndoAction::~MoveUndoAction() {
	for (GList * l = this ->originalPos; l != NULL; l = l->next) {
		MoveUndoEntry * u = (MoveUndoEntry *) l->data;
		delete u;
	}
	for (GList * l = this->newPos; l != NULL; l = l->next) {
		MoveUndoEntry * u = (MoveUndoEntry *) l->data;
		delete u;
	}
	g_list_free(this->originalPos);
}

void MoveUndoAction::finalize(EditSelection * selection) {
	for (GList * l = selection->selected; l != NULL; l = l->next) {
		Element * e = (Element *) l->data;
		this->newPos = g_list_append(this->newPos, new MoveUndoEntry(e,
				e->getX() + selection->x - selection->relativeX, e->getY() + selection->y - selection->relativeY));
	}

	if (this->page != selection->page) {
		this->newPage = selection->page;
		this->newLayer = selection->layer;
		this->newView = selection->view;
	}
}

bool MoveUndoAction::undo(Control * control) {
	if (this->oldLayer != this->newLayer && this->newLayer != NULL) {
		switchLayer(this->originalPos, this->newLayer, this->oldLayer);
	}

	acceptPositions(this->originalPos);

	repaint();

	return true;
}

bool MoveUndoAction::redo(Control * control) {
	if (this->oldLayer != this->newLayer && this->newLayer != NULL) {
		switchLayer(this->originalPos, this->oldLayer, this->newLayer);
	}

	acceptPositions(this->newPos);

	repaint();

	return true;
}

void MoveUndoAction::acceptPositions(GList * pos) {
	for (GList * l = pos; l != NULL; l = l->next) {
		MoveUndoEntry * u = (MoveUndoEntry *) l->data;
		Element * e = u->e;
		CHECK_MEMORY(e);

		e->move(u->x - e->getX(), u->y - e->getY());
		e->finalizeMove();
	}
}

void MoveUndoAction::switchLayer(GList * entries, Layer * oldLayer, Layer * newLayer) {
	for (GList * l = this->originalPos; l != NULL; l = l->next) {
		MoveUndoEntry * u = (MoveUndoEntry *) l->data;
		oldLayer->removeElement(u->e, false);
		newLayer->addElement(u->e);
	}
}

void MoveUndoAction::repaint() {
	if (!this->originalPos) {
		return;
	}

	//		MoveUndoEntry * u = (MoveUndoEntry *) this->originalPos->data;
	//		CHECK_MEMORY(u->e);
	//
	//		double x1 = u->e->getX();
	//		double x2 = u->e->getX() + u->e->getElementWidth();
	//		double y1 = u->e->getY();
	//		double y2 = u->e->getY() + u->e->getElementHeight();
	//
	//		for (GList * l = this->originalPos->next; l != NULL; l = l->next) {
	//			u = (MoveUndoEntry *) l->data;
	//			CHECK_MEMORY(u->e);
	//			x1 = MIN(x1, u->e->getX());
	//			x2 = MAX(x2, u->e->getX()+ u->e->getElementWidth());
	//			y1 = MIN(y1, u->e->getY());
	//			y2 = MAX(y2, u->e->getY()+ u->e->getElementHeight());
	//		}
	//
	//		CHECK_MEMORY(origView);
	//		origView->deleteViewBuffer();
	//		origView->redrawDocumentRegion(x1, y1, x2, y2);
	//
	//		if (newView) {
	//			CHECK_MEMORY(newView);
	//			newView->deleteViewBuffer();
	//			newView->redrawDocumentRegion(x1, y1, x2, y2);
	//		}

	origView->deleteViewBuffer();
	gtk_widget_queue_draw(origView->getWidget());
	if (newView) {
		CHECK_MEMORY(newView);
		newView->deleteViewBuffer();
		gtk_widget_queue_draw(newView->getWidget());
	}

}

XojPage ** MoveUndoAction::getPages() {
	XojPage ** pages = new XojPage *[3];
	pages[0] = this->page;
	pages[1] = this->newPage;
	pages[2] = NULL;
	return pages;
}

String MoveUndoAction::getText() {
	return _("Move");
}

//////////////////////////////////////////////////////////////////////

class SizeUndoActionEntry {
public:
	SizeUndoActionEntry(Stroke * s, double orignalWidth, double newWidth) {
		this->s = s;
		this->orignalWidth = orignalWidth;
		this->newWidth = newWidth;
	}

	Stroke * s;
	double orignalWidth;
	double newWidth;
};

SizeUndoAction::SizeUndoAction(XojPage * page, Layer * layer, Redrawable * view) {
	this->page = page;
	this->layer = layer;
	this->view = view;
	this->data = NULL;
}

SizeUndoAction::~SizeUndoAction() {
	for (GList * l = this->data; l != NULL; l = l->next) {
		SizeUndoActionEntry * e = (SizeUndoActionEntry *) l->data;
		delete e;
	}

	g_list_free(this->data);
}

void SizeUndoAction::addStroke(Stroke * s, double originalWidth, double newWidt) {
	this->data = g_list_append(this->data, new SizeUndoActionEntry(s, originalWidth, newWidt));
}

bool SizeUndoAction::undo(Control * control) {
	if (this->data == NULL) {
		return true;
	}

	SizeUndoActionEntry * e = (SizeUndoActionEntry *) this->data->data;
	double x1 = e->s->getX();
	double x2 = e->s->getX() + e->s->getElementWidth();
	double y1 = e->s->getY();
	double y2 = e->s->getY() + e->s->getElementHeight();

	for (GList * l = this->data; l != NULL; l = l->next) {
		SizeUndoActionEntry * e = (SizeUndoActionEntry *) l->data;
		e->s->setWidth(e->orignalWidth);

		x1 = MIN(x1, e->s->getX());
		x2 = MAX(x2, e->s->getX()+ e->s->getElementWidth());
		y1 = MIN(y1, e->s->getY());
		y2 = MAX(y2, e->s->getY()+ e->s->getElementHeight());
	}

	view->deleteViewBuffer();
	view->redrawDocumentRegion(x1, y1, x2, y2);

	return true;
}

bool SizeUndoAction::redo(Control * control) {
	if (this->data == NULL) {
		return true;
	}

	SizeUndoActionEntry * e = (SizeUndoActionEntry *) this->data->data;
	double x1 = e->s->getX();
	double x2 = e->s->getX() + e->s->getElementWidth();
	double y1 = e->s->getY();
	double y2 = e->s->getY() + e->s->getElementHeight();

	for (GList * l = this->data; l != NULL; l = l->next) {
		SizeUndoActionEntry * e = (SizeUndoActionEntry *) l->data;
		e->s->setWidth(e->newWidth);

		x1 = MIN(x1, e->s->getX());
		x2 = MAX(x2, e->s->getX()+ e->s->getElementWidth());
		y1 = MIN(y1, e->s->getY());
		y2 = MAX(y2, e->s->getY()+ e->s->getElementHeight());
	}

	view->deleteViewBuffer();
	view->redrawDocumentRegion(x1, y1, x2, y2);

	return true;
}

String SizeUndoAction::getText() {
	return _("Change stroke width");
}

//////////////////////////////////////////////////////////////////////

class ColorUndoActionEntry {
public:
	ColorUndoActionEntry(Element * e, int oldColor, int newColor) {
		this->e = e;
		this->oldColor = oldColor;
		this->newColor = newColor;
	}

	Element * e;
	int oldColor;
	int newColor;
};

ColorUndoAction::ColorUndoAction(XojPage * page, Layer * layer, Redrawable * view) {
	this->page = page;
	this->layer = layer;
	this->view = view;
	this->data = NULL;
}

ColorUndoAction::~ColorUndoAction() {
	for (GList * l = this->data; l != NULL; l = l->next) {
		ColorUndoActionEntry * e = (ColorUndoActionEntry *) l->data;
		delete e;
	}

	g_list_free(this->data);
}

void ColorUndoAction::addStroke(Element * e, int originalColor, double newColor) {
	this->data = g_list_append(this->data, new ColorUndoActionEntry(e, originalColor, newColor));
}

bool ColorUndoAction::undo(Control * control) {
	if (this->data == NULL) {
		return true;
	}

	ColorUndoActionEntry * e = (ColorUndoActionEntry *) this->data->data;
	double x1 = e->e->getX();
	double x2 = e->e->getX() + e->e->getElementWidth();
	double y1 = e->e->getY();
	double y2 = e->e->getY() + e->e->getElementHeight();

	for (GList * l = this->data; l != NULL; l = l->next) {
		ColorUndoActionEntry * e = (ColorUndoActionEntry *) l->data;
		e->e->setColor(e->oldColor);

		x1 = MIN(x1, e->e->getX());
		x2 = MAX(x2, e->e->getX()+ e->e->getElementWidth());
		y1 = MIN(y1, e->e->getY());
		y2 = MAX(y2, e->e->getY()+ e->e->getElementHeight());
	}

	view->deleteViewBuffer();
	view->redrawDocumentRegion(x1, y1, x2, y2);

	return true;
}

bool ColorUndoAction::redo(Control * control) {
	if (this->data == NULL) {
		return true;
	}

	ColorUndoActionEntry * e = (ColorUndoActionEntry *) this->data->data;
	double x1 = e->e->getX();
	double x2 = e->e->getX() + e->e->getElementWidth();
	double y1 = e->e->getY();
	double y2 = e->e->getY() + e->e->getElementHeight();

	for (GList * l = this->data; l != NULL; l = l->next) {
		ColorUndoActionEntry * e = (ColorUndoActionEntry *) l->data;
		e->e->setColor(e->newColor);

		x1 = MIN(x1, e->e->getX());
		x2 = MAX(x2, e->e->getX()+ e->e->getElementWidth());
		y1 = MIN(y1, e->e->getY());
		y2 = MAX(y2, e->e->getY()+ e->e->getElementHeight());
	}

	view->deleteViewBuffer();
	view->redrawDocumentRegion(x1, y1, x2, y2);

	return true;
}

String ColorUndoAction::getText() {
	return _("Change color");
}

//////////////////////////////////////////////////////////////////////

class FontUndoActionEntry {
public:
	FontUndoActionEntry(Text * e, XojFont & oldFont, XojFont & newFont) {
		this->e = e;
		this->oldFont = oldFont;
		this->newFont = newFont;
	}

	Text * e;
	XojFont oldFont;
	XojFont newFont;
};

FontUndoAction::FontUndoAction(XojPage * page, Layer * layer, Redrawable * view) {
	this->page = page;
	this->layer = layer;
	this->view = view;
	this->data = NULL;
}

FontUndoAction::~FontUndoAction() {
	for (GList * l = this->data; l != NULL; l = l->next) {
		FontUndoActionEntry * e = (FontUndoActionEntry *) l->data;
		delete e;
	}

	g_list_free(this->data);
}

void FontUndoAction::addStroke(Text * e, XojFont & oldFont, XojFont & newFont) {
	this->data = g_list_append(this->data, new FontUndoActionEntry(e, oldFont, newFont));
}

bool FontUndoAction::undo(Control * control) {
	if (this->data == NULL) {
		return true;
	}

	FontUndoActionEntry * e = (FontUndoActionEntry *) this->data->data;
	double x1 = e->e->getX();
	double x2 = e->e->getX() + e->e->getElementWidth();
	double y1 = e->e->getY();
	double y2 = e->e->getY() + e->e->getElementHeight();

	for (GList * l = this->data; l != NULL; l = l->next) {
		FontUndoActionEntry * e = (FontUndoActionEntry *) l->data;

		// size with old font
		x1 = MIN(x1, e->e->getX());
		x2 = MAX(x2, e->e->getX()+ e->e->getElementWidth());
		y1 = MIN(y1, e->e->getY());
		y2 = MAX(y2, e->e->getY()+ e->e->getElementHeight());

		e->e->setFont(e->oldFont);

		// size with new font
		x1 = MIN(x1, e->e->getX());
		x2 = MAX(x2, e->e->getX()+ e->e->getElementWidth());
		y1 = MIN(y1, e->e->getY());
		y2 = MAX(y2, e->e->getY()+ e->e->getElementHeight());
	}

	view->deleteViewBuffer();
	view->redrawDocumentRegion(x1, y1, x2, y2);

	return true;
}

bool FontUndoAction::redo(Control * control) {
	if (this->data == NULL) {
		return true;
	}

	FontUndoActionEntry * e = (FontUndoActionEntry *) this->data->data;
	double x1 = e->e->getX();
	double x2 = e->e->getX() + e->e->getElementWidth();
	double y1 = e->e->getY();
	double y2 = e->e->getY() + e->e->getElementHeight();

	for (GList * l = this->data; l != NULL; l = l->next) {
		FontUndoActionEntry * e = (FontUndoActionEntry *) l->data;

		// size with old font
		x1 = MIN(x1, e->e->getX());
		x2 = MAX(x2, e->e->getX()+ e->e->getElementWidth());
		y1 = MIN(y1, e->e->getY());
		y2 = MAX(y2, e->e->getY()+ e->e->getElementHeight());

		e->e->setFont(e->newFont);

		// size with new font
		x1 = MIN(x1, e->e->getX());
		x2 = MAX(x2, e->e->getX()+ e->e->getElementWidth());
		y1 = MIN(y1, e->e->getY());
		y2 = MAX(y2, e->e->getY()+ e->e->getElementHeight());
	}

	view->deleteViewBuffer();
	view->redrawDocumentRegion(x1, y1, x2, y2);

	return true;
}

String FontUndoAction::getText() {
	return _("Change font");
}

