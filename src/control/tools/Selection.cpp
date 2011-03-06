#include "Selection.h"

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

bool RectSelection::finalize(XojPage * page) {
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

void RectSelection::paint(cairo_t * cr, GdkEventExpose * event, double zoom) {
	GdkColor selectionColor = view->getSelectionColor();

	// set the line always the same size on display
	cairo_set_line_width(cr, 1 / zoom);
	gdk_cairo_set_source_color(cr, &selectionColor);

	cairo_move_to(cr, sx, sy);
	cairo_line_to(cr, ex, sy);
	cairo_line_to(cr, ex, ey);
	cairo_line_to(cr, sx, ey);
	cairo_close_path(cr);

	cairo_stroke_preserve(cr);
	cairo_set_source_rgba(cr, selectionColor.red / 65536.0, selectionColor.green / 65536.0, selectionColor.blue / 65536.0, 0.3);
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
		gdk_cairo_set_source_color(cr, &selectionColor);

		RegionPoint * r0 = (RegionPoint *) this->points->data;
		cairo_move_to(cr, r0->x, r0->y);

		for (GList * l = this->points->next; l != NULL; l = l->next) {
			RegionPoint * r = (RegionPoint *) l->data;
			cairo_line_to(cr, r->x, r->y);
		}

		cairo_line_to(cr, r0->x, r0->y);

		cairo_stroke_preserve(cr);
		cairo_set_source_rgba(cr, selectionColor.red / 65536.0, selectionColor.green / 65536.0, selectionColor.blue / 65536.0, 0.3);
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
	if (x < this->x1Box || x > this->x2Box) {
		return false;
	}
	if (y < this->y1Box || y > this->y2Box) {
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

bool RegionSelect::finalize(XojPage * page) {
	this->page = page;

	this->x1Box = 0;
	this->x2Box = 0;
	this->y1Box = 0;
	this->y2Box = 0;

	for (GList * l = this->points; l != NULL; l = l->next) {
		RegionPoint * p = (RegionPoint *) l->data;

		if (p->x < this->x1Box) {
			this->x1Box = p->x;
		} else if (p->x > this->x2Box) {
			this->x2Box = p->x;
		}

		if (p->y < this->y1Box) {
			this->y1Box = p->y;
		} else if (p->y > this->y2Box) {
			this->y2Box = p->y;
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

	view->redrawDocumentRegion(this->x1Box, this->y1Box, this->x2Box, this->y2Box);

	return this->selectedElements != NULL;
}

