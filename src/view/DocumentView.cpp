#include "DocumentView.h"

#include "TextView.h"

#include "gui/Cursor.h"
extern int currentToolType;

#include "background/MainBackgroundPainter.h"
#include "control/tools/EditSelection.h"
#include "control/tools/Selection.h"
#include "model/BackgroundImage.h"
#include "model/eraser/EraseableStroke.h"
#include "model/Layer.h"

#include <config.h>
#include <config-debug.h>

#include <gdk/gdk.h>

#include <typeinfo>

#ifdef DEBUG_SHOW_REPAINT_BOUNDS
#include <iostream>
using std::cout;
using std::endl;
#endif

DocumentView::DocumentView()
{
	XOJ_INIT_TYPE(DocumentView);
	this->page = NULL;
	this->cr = NULL;
	this->lX = -1;
	this->lY = -1;
	this->lWidth = -1;
	this->lHeight = -1;

	this->width = 0;
	this->height = 0;

	this->dontRenderEditingStroke = 0;
	this->backgroundPainter = new MainBackgroundPainter();
}

DocumentView::~DocumentView()
{
	delete this->backgroundPainter;
	this->backgroundPainter = NULL;

	XOJ_RELEASE_TYPE(DocumentView);
}

void DocumentView::applyColor(cairo_t* cr, Stroke* s)
{
	if (s->getToolType() == STROKE_TOOL_HIGHLIGHTER)
	{
		applyColor(cr, s, 120);
	}
	else
	{
		applyColor(cr, (Element*) s);
	}
}

void DocumentView::applyColor(cairo_t* cr, Element* e, int alpha)
{
	applyColor(cr, e->getColor(), alpha);
}

void DocumentView::applyColor(cairo_t* cr, int c, int alpha)
{
	double r = ((c >> 16) & 0xff) / 255.0;
	double g = ((c >> 8) & 0xff) / 255.0;
	double b = (c & 0xff) / 255.0;

	cairo_set_source_rgba(cr, r, g, b, alpha / 255.0);
}

void DocumentView::drawEraseableStroke(cairo_t* cr, Stroke* s)
{
	XOJ_CHECK_TYPE(DocumentView);

	EraseableStroke* e = s->getEraseable();

	e->draw(cr, this->lX, this->lY, this->width, this->height);
}

void DocumentView::drawStroke(cairo_t* cr, Stroke* s, int startPoint, double scaleFactor, bool changeSource)
{
	XOJ_CHECK_TYPE(DocumentView);

	ArrayIterator<Point> points = s->pointIterator();

	if (!points.hasNext())
	{
		// Should not happen
		g_warning("DocumentView::drawStroke empty stroke...");
		return;
	}

	if (changeSource)
	{
		if (s->getToolType() == STROKE_TOOL_HIGHLIGHTER ||
			(s->getAudioFilename().length() == 0 && currentToolType == TOOL_PLAY_OBJECT))
		{
			cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
			// Set the color
			applyColor(cr, s, 120);
		}
		else
		{
			cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
			// Set the color
			applyColor(cr, s);
		}
	}

	cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
	cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);

	// don't render eraseable for previews
	if (s->getEraseable() && !this->dontRenderEditingStroke)
	{
		drawEraseableStroke(cr, s);
		return;
	}

	int count = 1;
	double width = s->getWidth();

	// No pressure sensitivity, easy draw a line...
	if (!s->hasPressure())
	{
		// Set width
		cairo_set_line_width(cr, width * scaleFactor);

		while (points.hasNext())
		{
			Point p = points.next();

			if (startPoint <= count++)
			{
				cairo_line_to(cr, p.x, p.y);
			}
			else
			{
				cairo_move_to(cr, p.x, p.y);
			}
		}

		cairo_stroke(cr);
		return;
	}

	///////////////////////////////////////////////////////
	// Presure sensitiv stroken
	///////////////////////////////////////////////////////


	Point lastPoint1(-1, -1);
	lastPoint1 = points.next();

	while (points.hasNext())
	{
		Point p = points.next();
		if (startPoint <= count)
		{
			if (lastPoint1.z != Point::NO_PRESURE)
			{
				width = lastPoint1.z;
			}

			// Set width
			cairo_set_line_width(cr, width * scaleFactor);

			cairo_move_to(cr, lastPoint1.x, lastPoint1.y);
			cairo_line_to(cr, p.x, p.y);
			cairo_stroke(cr);
		}
		count++;
		lastPoint1 = p;
	}

	cairo_stroke(cr);
}

void DocumentView::drawText(cairo_t* cr, Text* t)
{
	XOJ_CHECK_TYPE(DocumentView);

	if (t->isInEditing())
	{
		return;
	}
	applyColor(cr, t);

	TextView::drawText(cr, t);
}

void DocumentView::drawImage(cairo_t* cr, Image* i)
{
	XOJ_CHECK_TYPE(DocumentView);

	cairo_matrix_t defaultMatrix = { 0 };
	cairo_get_matrix(cr, &defaultMatrix);

	cairo_surface_t* img = i->getImage();
	int width = cairo_image_surface_get_width(img);
	int height = cairo_image_surface_get_height(img);

	cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

	double xFactor = i->getElementWidth() / width;
	double yFactor = i->getElementHeight() / height;

	cairo_scale(cr, xFactor, yFactor);

	cairo_set_source_surface(cr, img, i->getX() / xFactor, i->getY() / yFactor);
	cairo_paint(cr);

	cairo_set_matrix(cr, &defaultMatrix);
}

void DocumentView::drawTexImage(cairo_t* cr, TexImage* i)
{
	XOJ_CHECK_TYPE(DocumentView);

	cairo_matrix_t defaultMatrix = { 0 };
	cairo_get_matrix(cr, &defaultMatrix);

	cairo_surface_t* img = i->getImage();
	int width = cairo_image_surface_get_width(img);
	int height = cairo_image_surface_get_height(img);

	cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

	double xFactor = i->getElementWidth() / width;
	double yFactor = i->getElementHeight() / height;

	cairo_scale(cr, xFactor, yFactor);

	cairo_set_source_surface(cr, img, i->getX() / xFactor, i->getY() / yFactor);
	cairo_paint(cr);

	cairo_set_matrix(cr, &defaultMatrix);
}

void DocumentView::drawElement(cairo_t* cr, Element* e)
{
	XOJ_CHECK_TYPE(DocumentView);

	if (e->getType() == ELEMENT_STROKE)
	{
		drawStroke(cr, (Stroke*) e);
	}
	else if (e->getType() == ELEMENT_TEXT)
	{
		drawText(cr, (Text*) e);
	}
	else if (e->getType() == ELEMENT_IMAGE)
	{
		drawImage(cr, (Image*) e);
	}
	else if (e->getType() == ELEMENT_TEXIMAGE)
	{
		drawTexImage(cr, (TexImage*) e);
	}
}

/**
 * Draw a single layer
 * @param cr Draw to thgis context
 * @param l The layer to draw
 */
void DocumentView::drawLayer(cairo_t* cr, Layer* l)
{
	XOJ_CHECK_TYPE(DocumentView);

	cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);

#ifdef DEBUG_SHOW_REPAINT_BOUNDS
	int drawed = 0;
	int notDrawed = 0;
#endif // DEBUG_SHOW_REPAINT_BOUNDS
	for (Element* e : *l->getElements())
	{
#ifdef DEBUG_SHOW_ELEMENT_BOUNDS
		cairo_set_source_rgb(cr, 0, 1, 0);
		cairo_set_line_width(cr, 1);
		cairo_rectangle(cr, e->getX(), e->getY(), e->getElementWidth(), e->getElementHeight());
		cairo_stroke(cr);
#endif // DEBUG_SHOW_REPAINT_BOUNDS
		//cairo_new_path(cr);

		if (this->lX != -1)
		{
			if (e->intersectsArea(this->lX, this->lY, this->width, this->height))
			{
				drawElement(cr, e);
#ifdef DEBUG_SHOW_REPAINT_BOUNDS
				drawed++;
#endif // DEBUG_SHOW_REPAINT_BOUNDS
			}
#ifdef DEBUG_SHOW_REPAINT_BOUNDS
			else
			{
				notDrawed++;
			}
#endif // DEBUG_SHOW_REPAINT_BOUNDS

		}
		else
		{
#ifdef DEBUG_SHOW_REPAINT_BOUNDS
			drawed++;
#endif // DEBUG_SHOW_REPAINT_BOUNDS
			drawElement(cr, e);
		}
	}

#ifdef DEBUG_SHOW_REPAINT_BOUNDS
	cout << bl::format("DBG:DocumentView: draw {1} / not draw {2}") % drawed % notDrawed << endl;
#endif // DEBUG_SHOW_REPAINT_BOUNDS
}

void DocumentView::paintBackgroundImage()
{
	XOJ_CHECK_TYPE(DocumentView);

	GdkPixbuf* pixbuff = page->getBackgroundImage().getPixbuf();
	if (pixbuff)
	{
		cairo_matrix_t matrix = { 0 };
		cairo_get_matrix(cr, &matrix);

		int width = gdk_pixbuf_get_width(pixbuff);
		int height = gdk_pixbuf_get_height(pixbuff);

		double sx = page->getWidth() / width;
		double sy = page->getHeight() / height;

		cairo_scale(cr, sx, sy);

		gdk_cairo_set_source_pixbuf(cr, pixbuff, 0, 0);
		cairo_paint(cr);

		cairo_set_matrix(cr, &matrix);
	}
}

void DocumentView::drawSelection(cairo_t* cr, ElementContainer* container)
{
	XOJ_CHECK_TYPE(DocumentView);

	for (Element* e : *container->getElements())
	{
		drawElement(cr, e);
	}
}

void DocumentView::limitArea(double x, double y, double width, double height)
{
	XOJ_CHECK_TYPE(DocumentView);

	this->lX = x;
	this->lY = y;
	this->lWidth = width;
	this->lHeight = height;
}

/**
 * Drawing first step
 * @param page The page to draw
 * @param cr Draw to thgis context
 * @param dontRenderEditingStroke false to draw currently drawing stroke
 */
void DocumentView::initDrawing(PageRef page, cairo_t* cr, bool dontRenderEditingStroke)
{
	XOJ_CHECK_TYPE(DocumentView);

	this->cr = cr;
	this->page = page;
	this->width = page->getWidth();
	this->height = page->getHeight();
	this->dontRenderEditingStroke = dontRenderEditingStroke;
}

/**
 * Last step in drawing
 */
void DocumentView::finializeDrawing()
{
	XOJ_CHECK_TYPE(DocumentView);

#ifdef DEBUG_SHOW_REPAINT_BOUNDS
	if (this->lX != -1)
	{
		cout << "DBG:repaint area" << endl;
		cairo_set_source_rgb(cr, 1, 0, 0);
		cairo_set_line_width(cr, 1);
		cairo_rectangle(cr, this->lX + 3, this->lY + 3, this->lWidth - 6, this->lHeight - 6);
		cairo_stroke(cr);
	}
	else
	{
		cout << "DBG:repaint complete" << endl;
	}
#endif // DEBUG_SHOW_REPAINT_BOUNDS

	this->lX = -1;
	this->lY = -1;
	this->lWidth = -1;
	this->lHeight = -1;

	this->page = NULL;
	this->cr = NULL;
}

/**
 * Draw the background
 */
void DocumentView::drawBackground()
{
	PageType pt = page->getBackgroundType();
	if (pt.isPdfPage())
	{
		// Handled in PdfView
	}
	else if (pt.isImagePage())
	{
		paintBackgroundImage();
	}
	else
	{
		backgroundPainter->paint(pt, cr, page);
	}
}

/**
 * Draw the full page, usually you would like to call this method
 * @param page The page to draw
 * @param cr Draw to thgis context
 * @param dontRenderEditingStroke false to draw currently drawing stroke
 * @param hideBackground true to hide the background
 */
void DocumentView::drawPage(PageRef page, cairo_t* cr, bool dontRenderEditingStroke, bool hideBackground)
{
	XOJ_CHECK_TYPE(DocumentView);

	initDrawing(page, cr, dontRenderEditingStroke);

	if (!hideBackground)
	{
		drawBackground();
	}

	int layer = 0;
	for (Layer* l : *page->getLayers())
	{
		if (l == NULL)
		{
			break;
		}

		if (layer >= page->getSelectedLayerId())
		{
			break;
		}

		drawLayer(cr, l);
		layer++;
	}

	finializeDrawing();
}
