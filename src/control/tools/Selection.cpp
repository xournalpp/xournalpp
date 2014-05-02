#include "Selection.h"
#include "../../model/Layer.h"

Selection::Selection(Redrawable* view)
{
	XOJ_INIT_TYPE(Selection);

	this->view = view;
	this->selectedElements = NULL;
	this->page = NULL;
}

Selection::~Selection()
{
	XOJ_CHECK_TYPE(Selection);

	this->view = NULL;
	this->page = NULL;
	g_list_free(this->selectedElements);
	this->selectedElements = NULL;

	XOJ_RELEASE_TYPE(Selection);
}

void Selection::getSelectedRect(double& x, double& y, double& width,
                                double& height)
{
	XOJ_CHECK_TYPE(Selection);

	if (this->selectedElements == NULL)
	{
		x = 0;
		y = 0;
		width = 0;
		height = 0;
		return;
	}

	Element* first = (Element*) this->selectedElements->data;
	Range range(first->getX(), first->getY());

	for (GList* l = this->selectedElements; l != NULL; l = l->next)
	{
		Element* e = (Element*) l->data;

		range.addPoint(e->getX(), e->getY());
		range.addPoint(e->getX() + e->getElementWidth(),
		               e->getY() + e->getElementHeight());
	}

	x = range.getX() - 3;
	y = range.getY() - 3;
	width = range.getWidth() + 6;
	height = range.getHeight() + 6;
}

//////////////////////////////////////////////////////////

RectSelection::RectSelection(double x, double y, Redrawable* view) :
	Selection(view)
{
	XOJ_INIT_TYPE(RectSelection);

	this->sx = x;
	this->sy = y;
	this->ex = x;
	this->ey = y;
	this->x1 = 0;
	this->x2 = 0;
	this->y1 = 0;
	this->y2 = 0;
}

RectSelection::~RectSelection()
{
	XOJ_RELEASE_TYPE(RectSelection);
}

bool RectSelection::finalize(PageRef page)
{
	XOJ_CHECK_TYPE(RectSelection);

	this->x1 = MIN(this->sx, this->ex);
	this->x2 = MAX(this->sx, this->ex);

	this->y1 = MIN(this->sy, this->ey);
	this->y2 = MAX(this->sy, this->ey);

	this->page = page;

	Layer* l = page->getSelectedLayer();
	ListIterator<Element*> eit = l->elementIterator();
	while (eit.hasNext())
	{
		Element* e = eit.next();
		if (e->isInSelection(this))
		{
			this->selectedElements = g_list_append(this->selectedElements, e);
		}
	}

	view->repaintArea(this->x1 - 10, this->y1 - 10,
	                  this->x2 + 10, this->y2 + 10);

	return this->selectedElements != NULL;
}

bool RectSelection::contains(double x, double y)
{
	XOJ_CHECK_TYPE(RectSelection);

	if (x < this->x1 || x > this->x2)
	{
		return false;
	}
	if (y < this->y1 || y > this->y2)
	{
		return false;
	}

	return true;
}

void RectSelection::currentPos(double x, double y)
{
	XOJ_CHECK_TYPE(RectSelection);

	int aX = MIN(x, this->ex);
	aX = MIN(aX, this->sx) - 10;

	int bX = MAX(x, this->ex);
	bX = MAX(bX , this->sx) + 10;

	int aY = MIN(y, this->ey);
	aY = MIN(aY, this->sy) - 10;

	int bY = MAX(y, this->ey);
	bY = MAX(bY, this->sy) + 10;

	view->repaintArea(aX, aY, bX, bY);

	this->ex = x;
	this->ey = y;
}

void RectSelection::paint(cairo_t* cr, GdkRectangle* rect, double zoom)
{
	XOJ_CHECK_TYPE(RectSelection);

	GdkColor selectionColor = view->getSelectionColor();

	// set the line always the same size on display
	cairo_set_line_width(cr, 1 / zoom);
	gdk_threads_enter();
	gdk_cairo_set_source_color(cr, &selectionColor);
	gdk_threads_leave();


	int aX = MIN(this->sx, this->ex);
	int bX = MAX(this->sx, this->ex);

	int aY = MIN(this->sy, this->ey);
	int bY = MAX(this->sy, this->ey);

	cairo_move_to(cr, aX, aY);
	cairo_line_to(cr, bX, aY);
	cairo_line_to(cr, bX, bY);
	cairo_line_to(cr, aX, bY);
	cairo_close_path(cr);

	cairo_stroke_preserve(cr);
	cairo_set_source_rgba(cr, selectionColor.red / 65536.0,
	                      selectionColor.green / 65536.0, selectionColor.blue / 65536.0, 0.3);
	cairo_fill(cr);
}

//////////////////////////////////////////////////////////


class RegionPoint
{
public:
	RegionPoint(double x, double y)
	{
		this->x = x;
		this->y = y;
	}

	double x;
	double y;
};

RegionSelect::RegionSelect(double x, double y, Redrawable* view) :
	Selection(view)
{
	XOJ_INIT_TYPE(RegionSelect);

	this->points = NULL;
	currentPos(x, y);
}

RegionSelect::~RegionSelect()
{
	XOJ_CHECK_TYPE(RegionSelect);

	for (GList* l = this->points; l != NULL; l = l->next)
	{
		delete (RegionPoint*) l->data;
	}
	g_list_free(this->points);

	XOJ_RELEASE_TYPE(RegionSelect);
}

void RegionSelect::paint(cairo_t* cr, GdkRectangle* rect, double zoom)
{
	XOJ_CHECK_TYPE(RegionSelect);

	// at least three points needed
	if (this->points && this->points->next && this->points->next->next)
	{
		GdkColor selectionColor = view->getSelectionColor();

		// set the line always the same size on display
		cairo_set_line_width(cr, 1 / zoom);
		gdk_threads_enter();
		gdk_cairo_set_source_color(cr, &selectionColor);
		gdk_threads_leave();

		RegionPoint* r0 = (RegionPoint*) this->points->data;
		cairo_move_to(cr, r0->x, r0->y);

		for (GList* l = this->points->next; l != NULL; l = l->next)
		{
			RegionPoint* r = (RegionPoint*) l->data;
			cairo_line_to(cr, r->x, r->y);
		}

		cairo_line_to(cr, r0->x, r0->y);

		cairo_stroke_preserve(cr);
		cairo_set_source_rgba(cr, selectionColor.red / 65536.0,
		                      selectionColor.green / 65536.0, selectionColor.blue / 65536.0, 0.3);
		cairo_fill(cr);
	}
}

void RegionSelect::currentPos(double x, double y)
{
	XOJ_CHECK_TYPE(RegionSelect);

	this->points = g_list_append(this->points, new RegionPoint(x, y));

	// at least three points needed
	if (this->points && this->points->next && this->points->next->next)
	{

		RegionPoint* r0 = (RegionPoint*) this->points->data;
		double ax = r0->x;
		double bx = r0->x;
		double ay = r0->y;
		double by = r0->y;

		for (GList* l = this->points; l != NULL; l = l->next)
		{
			RegionPoint* r = (RegionPoint*) l->data;
			if (ax > r->x)
			{
				ax = r->x;
			}
			if (bx < r->x)
			{
				bx = r->x;
			}
			if (ay > r->y)
			{
				ay = r->y;
			}
			if (by < r->y)
			{
				by = r->y;
			}
		}

		view->repaintArea(ax, ay, bx, by);
	}
}

bool RegionSelect::contains(double x, double y)
{
	XOJ_CHECK_TYPE(RegionSelect);

	if (x < this->x1Box || x > this->x2Box)
	{
		return false;
	}
	if (y < this->y1Box || y > this->y2Box)
	{
		return false;
	}
	if (this->points == NULL || this->points->next == NULL)
	{
		return false;
	}

	int hits = 0;

	RegionPoint* last = (RegionPoint*) g_list_last(this->points)->data;

	double lastx = last->x;
	double lasty = last->y;
	double curx, cury;

	// Walk the edges of the polygon
	for (GList* l = this->points; l != NULL;
	     lastx = curx, lasty = cury, l = l->next)
	{
		RegionPoint* last = (RegionPoint*) l->data;
		curx = last->x;
		cury = last->y;

		if (cury == lasty)
		{
			continue;
		}

		int leftx;
		if (curx < lastx)
		{
			if (x >= lastx)
			{
				continue;
			}
			leftx = curx;
		}
		else
		{
			if (x >= curx)
			{
				continue;
			}
			leftx = lastx;
		}

		double test1, test2;
		if (cury < lasty)
		{
			if (y < cury || y >= lasty)
			{
				continue;
			}
			if (x < leftx)
			{
				hits++;
				continue;
			}
			test1 = x - curx;
			test2 = y - cury;
		}
		else
		{
			if (y < lasty || y >= cury)
			{
				continue;
			}
			if (x < leftx)
			{
				hits++;
				continue;
			}
			test1 = x - lastx;
			test2 = y - lasty;
		}

		if (test1 < (test2 / (lasty - cury) * (lastx - curx)))
		{
			hits++;
		}
	}

	return ((hits & 1) != 0);
}

bool RegionSelect::finalize(PageRef page)
{
	XOJ_CHECK_TYPE(RegionSelect);

	this->page = page;

	this->x1Box = 0;
	this->x2Box = 0;
	this->y1Box = 0;
	this->y2Box = 0;

	for (GList* l = this->points; l != NULL; l = l->next)
	{
		RegionPoint* p = (RegionPoint*) l->data;

		if (p->x < this->x1Box)
		{
			this->x1Box = p->x;
		}
		else if (p->x > this->x2Box)
		{
			this->x2Box = p->x;
		}

		if (p->y < this->y1Box)
		{
			this->y1Box = p->y;
		}
		else if (p->y > this->y2Box)
		{
			this->y2Box = p->y;
		}
	}

	Layer* l = page->getSelectedLayer();
	ListIterator<Element*> eit = l->elementIterator();
	while (eit.hasNext())
	{
		Element* e = eit.next();
		if (e->isInSelection(this))
		{
			this->selectedElements = g_list_append(this->selectedElements, e);
		}
	}

	view->repaintArea(this->x1Box - 10, this->y1Box - 10,
	                  this->x2Box + 10, this->y2Box + 10);

	return this->selectedElements != NULL;
}

