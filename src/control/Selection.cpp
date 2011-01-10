#include "Selection.h"
#include "../gui/XournalWidget.h"
#include "../gettext.h"

Selection::Selection(Redrawable * view) {
	this->view = view;
	this->selectedElements = NULL;
	this->page = NULL;
}

Selection::~Selection() {
	this->view = NULL;
	this->page = NULL;
	g_list_free(this->selectedElements);
	this->selectedElements = NULL;
}

//////////////////////////////////////////////////////////

RectSelection::RectSelection(double x, double y, Redrawable * view) :
	Selection(view) {
	this->sx = x;
	this->sy = y;
	this->ex = x;
	this->ey = y;
	this->x1 = 0;
	this->x2 = 0;
	this->y1 = 0;
	this->y2 = 0;
}

void RectSelection::getSelectedRect(double & x, double & y, double & width, double & height) {
	x = this->sx;
	y = this->sy;
	width = this->ex - this->sx;
	height = this->ey - this->sy;
}

bool RectSelection::finnalize(XojPage * page) {
	x1 = MIN(this->sx, this->ex);
	x2 = MAX(this->sx, this->ex);

	y1 = MIN(this->sy, this->ey);
	y2 = MAX(this->sy, this->ey);

	this->page = page;

	Layer * l = page->getSelectedLayer();
	ListIterator<Element *> eit = l->elementIterator();
	while (eit.hasNext()) {
		Element * e = eit.next();
		if (e->isInSelection(this)) {
			this->selectedElements = g_list_append(this->selectedElements, e);
		}
	}

	view->redrawDocumentRegion(x1, y1, x2, y2);

	return this->selectedElements != NULL;
}

bool RectSelection::contains(double x, double y) {
	if (x < this->x1 || x > this->x2) {
		return false;
	}
	if (y < this->y1 || y > this->y2) {
		return false;
	}

	return true;
}

void RectSelection::currentPos(double x, double y) {
	int aX = MIN(x, this->ex);
	aX = MIN(aX, this->sx) - 10;

	int bX = MAX(x, this->ex);
	bX = MAX(bX , this->sx) + 10;

	int aY = MIN(y, this->ey);
	aY = MIN(aY, this->sy) - 10;

	int bY = MAX(y, this->ey);
	bY = MAX(bY, this->sy) + 10;

	if (x <= this->sx) {
		this->sx = x;
	} else {
		this->ex = x;
	}

	if (y <= this->sy) {
		this->sy = y;
	} else {
		this->ey = y;
	}

	view->redrawDocumentRegion(aX, aY, bX, bY);
}

void RectSelection::paint(cairo_t * cr, GdkEventExpose *event, double zoom) {
	GdkColor selectionColor = view->getSelectionColor();

	// set the line always the same size on display
	cairo_set_line_width(cr, 1 / zoom);
	cairo_set_source_rgb(cr, selectionColor.red / 65536.0, selectionColor.green / 65536.0, selectionColor.blue
			/ 65536.0);

	cairo_move_to(cr, sx, sy);
	cairo_line_to(cr, ex, sy);
	cairo_line_to(cr, ex, ey);
	cairo_line_to(cr, sx, ey);
	cairo_close_path(cr);

	cairo_stroke_preserve(cr);
	cairo_set_source_rgba(cr, selectionColor.red / 65536.0, selectionColor.green / 65536.0, selectionColor.blue
			/ 65536.0, 0.3);
	cairo_fill(cr);
}

//////////////////////////////////////////////////////////


class RegionPoint {
public:
	RegionPoint(double x, double y) {
		this->x = x;
		this->y = y;
	}

	double x;
	double y;
};

RegionSelect::RegionSelect(double x, double y, Redrawable * view) :
	Selection(view) {
	this->points = NULL;
	currentPos(x, y);
}

void RegionSelect::getSelectedRect(double & x, double & y, double & width, double & height) {
	if (this->selectedElements == NULL) {
		x = 0;
		y = 0;
		width = 0;
		height = 0;
		return;
	}

	Element * first = (Element *) this->selectedElements->data;

	double aX = first->getX();
	double bX = first->getX();
	double aY = first->getY();
	double bY = first->getY();

	for (GList * l = this->selectedElements; l != NULL; l = l->next) {
		Element * e = (Element *) l->data;
		if (aX > e->getX()) {
			aX = e->getX();
		}
		if (aY > e->getY()) {
			aY = e->getY();
		}

		if (bX < e->getX() + e->getElementWidth()) {
			bX = e->getX() + e->getElementWidth();
		}
		if (bY < e->getY() + e->getElementHeight()) {
			bY = e->getY() + e->getElementHeight();
		}
	}

	x = aX - 3;
	y = aY - 3;
	width = bX - aX + 6;
	height = bY - aY + 6;
}

RegionSelect::~RegionSelect() {
	for (GList * l = this->points; l != NULL; l = l->next) {
		delete (RegionPoint *) l->data;
	}
	g_list_free(this->points);
}

void RegionSelect::paint(cairo_t * cr, GdkEventExpose *event, double zoom) {
	// at least three points needed
	if (this->points && this->points->next && this->points->next->next) {
		GdkColor selectionColor = view->getSelectionColor();

		// set the line always the same size on display
		cairo_set_line_width(cr, 1 / zoom);
		cairo_set_source_rgb(cr, selectionColor.red / 65536.0, selectionColor.green / 65536.0, selectionColor.blue
				/ 65536.0);

		RegionPoint * r0 = (RegionPoint *) this->points->data;
		cairo_move_to(cr, r0->x, r0->y);

		for (GList * l = this->points->next; l != NULL; l = l->next) {
			RegionPoint * r = (RegionPoint *) l->data;
			cairo_line_to(cr, r->x, r->y);
		}

		cairo_line_to(cr, r0->x, r0->y);

		cairo_stroke_preserve(cr);
		cairo_set_source_rgba(cr, selectionColor.red / 65536.0, selectionColor.green / 65536.0, selectionColor.blue
				/ 65536.0, 0.3);
		cairo_fill(cr);
	}
}

void RegionSelect::currentPos(double x, double y) {
	this->points = g_list_append(this->points, new RegionPoint(x, y));

	// at least three points needed
	if (this->points && this->points->next && this->points->next->next) {

		RegionPoint * r0 = (RegionPoint *) this->points->data;
		double ax = r0->x;
		double bx = r0->x;
		double ay = r0->y;
		double by = r0->y;

		for (GList * l = this->points; l != NULL; l = l->next) {
			RegionPoint * r = (RegionPoint *) l->data;
			if (ax > r->x) {
				ax = r->x;
			}
			if (bx < r->x) {
				bx = r->x;
			}
			if (ay > r->y) {
				ay = r->y;
			}
			if (by < r->y) {
				by = r->y;
			}
		}

		view->redrawDocumentRegion(ax, ay, bx, by);
	}
}

bool RegionSelect::contains(double x, double y) {
	if (x < x1Box || x > x2Box) {
		return false;
	}
	if (y < y1Box || y > y2Box) {
		return false;
	}
	if (this->points == NULL || this->points->next == NULL) {
		return false;
	}

	int hits = 0;

	RegionPoint * last = (RegionPoint *) g_list_last(this->points)->data;

	double lastx = last->x;
	double lasty = last->y;
	double curx, cury;

	// Walk the edges of the polygon
	for (GList * l = this->points; l != NULL; lastx = curx, lasty = cury, l = l->next) {
		RegionPoint * last = (RegionPoint *) l->data;
		curx = last->x;
		cury = last->y;

		if (cury == lasty) {
			continue;
		}

		int leftx;
		if (curx < lastx) {
			if (x >= lastx) {
				continue;
			}
			leftx = curx;
		} else {
			if (x >= curx) {
				continue;
			}
			leftx = lastx;
		}

		double test1, test2;
		if (cury < lasty) {
			if (y < cury || y >= lasty) {
				continue;
			}
			if (x < leftx) {
				hits++;
				continue;
			}
			test1 = x - curx;
			test2 = y - cury;
		} else {
			if (y < lasty || y >= cury) {
				continue;
			}
			if (x < leftx) {
				hits++;
				continue;
			}
			test1 = x - lastx;
			test2 = y - lasty;
		}

		if (test1 < (test2 / (lasty - cury) * (lastx - curx))) {
			hits++;
		}
	}

	return ((hits & 1) != 0);
}

bool RegionSelect::finnalize(XojPage * page) {
	this->page = page;

	x1Box = 0;
	x2Box = 0;
	y1Box = 0;
	y2Box = 0;

	for (GList * l = this->points; l != NULL; l = l->next) {
		RegionPoint * p = (RegionPoint *) l->data;

		if (p->x < x1Box) {
			x1Box = p->x;
		} else if (p->x > x2Box) {
			x2Box = p->x;
		}

		if (p->y < y1Box) {
			y1Box = p->y;
		} else if (p->y > y2Box) {
			y2Box = p->y;
		}
	}

	Layer * l = page->getSelectedLayer();
	ListIterator<Element *> eit = l->elementIterator();
	while (eit.hasNext()) {
		Element * e = eit.next();
		if (e->isInSelection(this)) {
			this->selectedElements = g_list_append(this->selectedElements, e);
		}
	}

	view->redrawDocumentRegion(x1Box, y1Box, x2Box, y2Box);

	return this->selectedElements != NULL;
}

//////////////////////////////////////////////////////////

EditSelection::EditSelection(Selection * selection, Redrawable * view) {
	selection->getSelectedRect(this->x, this->y, this->width, this->height);
	this->selected = g_list_copy(selection->selectedElements);
	this->page = selection->page;
	this->view = view;
	this->inputView = view;

	this->selType = CURSOR_SELECTION_NONE;
	this->selX = 0;
	this->selY = 0;

	this->undo = NULL;
}

EditSelection::EditSelection(Element * e, Redrawable * view, XojPage * page) {
	this->selected = g_list_append(NULL, e);
	this->page = page;
	this->view = view;
	this->inputView = view;

	this->x = e->getX();
	this->y = e->getY();
	this->width = e->getElementWidth();
	this->height = e->getElementHeight();

	this->selType = CURSOR_SELECTION_NONE;
	this->selX = 0;
	this->selY = 0;

	this->undo = NULL;
}

EditSelection::~EditSelection() {
	view->redrawDocumentRegion(x, y, x + width, y + height);
	g_list_free(this->selected);
}

void EditSelection::finalizeEditing() {
	this->selX = 0;
	this->selY = 0;

	if (this->selType == CURSOR_SELECTION_MOVE) {
		for (GList * l = this->selected; l != NULL; l = l->next) {
			Element * e = (Element *) l->data;
			e->finalizeMove();
		}
	}

	this->selType = CURSOR_SELECTION_NONE;

	this->inputView = this->view;

	if (this->undo) {
		this->undo->finalize(this);
	}
}

MoveUndoAction * EditSelection::getUndoAction() {
	MoveUndoAction * u = this->undo;
	this->undo = NULL;
	return u;
}

GList * EditSelection::getElements() {
	return this->selected;
}

void EditSelection::setEditMode(CursorSelectionType selType, double x, double y) {
	this->selType = selType;
	this->selX = x;
	this->selY = y;
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
	gtk_widget_get_allocation(view->getWidget(), &alloc);

	this->x += dx;
	this->y += dy;

	double zoom = xournal->getZoom();

	// Test if the selection is moved to another page
	int xPos = x * zoom + alloc.x;
	int yPos = y * zoom + alloc.y;

	PageView * v = xournal->getViewAt(xPos, yPos);
	Redrawable * lastView = NULL;
	if (v != NULL && this->view != v) {
		moveSelectionToPage(v->getPage());
		lastView = this->view;
		this->view = v;

		GtkAllocation alloc1 = { 0 };
		gtk_widget_get_allocation(lastView->getWidget(), &alloc1);
		GtkAllocation alloc2 = { 0 };
		gtk_widget_get_allocation(this->view->getWidget(), &alloc2);

		double zoom = xournal->getZoom();

		double xOffset = alloc2.x - alloc1.x;
		double yOffset = alloc2.y - alloc1.y;

		if (xOffset > 1 || xOffset < -1) {
			if (xOffset > 1) {
				xOffset = page->getWidth() + XOURNAL_PADDING / zoom;
			} else {
				xOffset = -page->getWidth() - XOURNAL_PADDING / zoom;
			}
		}
		if (yOffset > 1 || yOffset < -1) {
			if (yOffset > 1) {
				yOffset = page->getHeight() + XOURNAL_PADDING / zoom;
			} else {
				yOffset = -page->getHeight() - XOURNAL_PADDING / zoom;
			}
		}

		this->x -= xOffset;
		this->y -= yOffset;

		dx -= xOffset;
		dy -= yOffset;

		page = v->getPage();
	}

	x1 = MIN(x1, this->x);
	y1 = MIN(y1, this->y);

	x2 = MAX(x2, this->x + this->width);
	y2 = MAX(y2, this->y + this->height);

	for (GList * l = this->selected; l != NULL; l = l->next) {
		Element * e = (Element *) l->data;
		e->move(dx, dy);
	}

	if (lastView) {
		lastView->deleteViewBuffer();
		lastView->redrawDocumentRegion(x1, y1, x2, y2);
	}
	this->view->deleteViewBuffer();
	this->view->redrawDocumentRegion(x1, y1, x2, y2);
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

void EditSelection::moveSelectionToPage(XojPage * page) {
	for (GList * l = this->selected; l != NULL; l = l->next) {
		Element * e = (Element *) l->data;
		this->page->getSelectedLayer()->removeElement(e, false);
		page->getSelectedLayer()->addElement(e);
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
	this->oldLayer = page->getSelectedLayer();
	this->newLayer = NULL;
	this->origView = selection->view;
	this->newView = NULL;

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
		this->newPos = g_list_append(this->newPos, new MoveUndoEntry(e, e->getX(), e->getY()));
	}

	if (this->page != selection->page) {
		this->newPage = selection->page;
		this->newLayer = this->newPage->getSelectedLayer();
		this->newView = selection->view;
	}
}

bool MoveUndoAction::undo(Control * control) {
	if (this->oldLayer != this->newLayer) {
		switchLayer(this->originalPos, this->newLayer, this->oldLayer);
	}

	acceptPositions(this->originalPos);

	repaint();

	return true;
}

bool MoveUndoAction::redo(Control * control) {
	if (this->oldLayer != this->newLayer) {
		switchLayer(this->originalPos, this->oldLayer, this->newLayer);
	}

	acceptPositions(this->newPos);

	repaint();

	return true;
}

void MoveUndoAction::acceptPositions(GList * pos) {
	for (GList * l = pos; l != NULL; l = l->next) {
		MoveUndoEntry * u = (MoveUndoEntry *) l->data;
		u->e->setX(u->x);
		u->e->setY(u->y);
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
	Element * e = NULL;

	if (!this->originalPos) {
		return;
	}

	e = (Element *) this->originalPos->data;

	double x1 = e->getX();
	double x2 = e->getX() + e->getElementWidth();
	double y1 = e->getY();
	double y2 = e->getY() + e->getElementHeight();

	for (GList * l = this->originalPos->next; l != NULL; l = l->next) {
		e = (Element *) l->data;
		x1 = MIN(x1, e->getX());
		x2 = MAX(x2, e->getX()+ e->getElementWidth());
		y1 = MIN(y1, e->getY());
		y2 = MAX(y2, e->getY()+ e->getElementHeight());
	}

	origView->deleteViewBuffer();
	origView->redrawDocumentRegion(x1, y1, x2, y2);

	if (newView) {
		newView->deleteViewBuffer();
		newView->redrawDocumentRegion(x1, y1, x2, y2);
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

