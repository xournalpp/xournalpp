#include "DocumentView.h"
#include "TextView.h"
#include "../gettext.h"
#include <gdk/gdk.h>
#include "../control/tools/Selection.h"
#include "../control/tools/EditSelection.h"
#include "../model/EraseableStroke.h"

//#define SHOW_ELEMENT_BOUNDS
//#define SHOW_REPAINT_BOUNDS

DocumentView::DocumentView() {
	this->page = NULL;
	this->cr = NULL;
	this->lX = -1;
	this->lY = -1;
	this->lWidth = -1;
	this->lHeight = -1;
}

DocumentView::~DocumentView() {
}

void DocumentView::applyColor(cairo_t * cr, Element * e, int alpha) {
	applyColor(cr, e->getColor(), alpha);
}

void DocumentView::applyColor(cairo_t * cr, int c, int alpha) {
	double r = ((c >> 16) & 0xff) / 255.0;
	double g = ((c >> 8) & 0xff) / 255.0;
	double b = (c & 0xff) / 255.0;

	cairo_set_source_rgba(cr, r, g, b, alpha / 255.0);
}

void DocumentView::drawEraseableStroke(cairo_t * cr, Stroke * s) {
	EraseableStroke * e = s->getEraseable();

	double width = s->getWidth();

	for (GList * l = e->getParts(); l != NULL; l = l->next) {
		EraseableStrokePart * part = (EraseableStrokePart *) l->data;
		if (part->getWidth() == Point::NO_PRESURE) {
			cairo_set_line_width(cr, width);
		} else {
			cairo_set_line_width(cr, part->getWidth());
		}

		GList * pl = part->getPoints();
		Point * p = (Point *) pl->data;
		cairo_move_to(cr, p->x, p->y);

		pl = pl->next;
		for (; pl != NULL; pl = pl->next) {
			Point * p = (Point *) pl->data;
			cairo_line_to(cr, p->x, p->y);
		}
		cairo_stroke(cr);
	}
}

void DocumentView::drawStroke(cairo_t * cr, Stroke * s, int startPoint) {
	ArrayIterator<Point> points = s->pointIterator();

	if (!points.hasNext()) {
		// Empty stroke... Should not happen
		return;
	}

	if (s->getToolType() == STROKE_TOOL_HIGHLIGHTER) {
		cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
		// Set the color
		applyColor(cr, s, 120);
	} else {
		cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
		// Set the color
		applyColor(cr, s);
	}

	cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
	cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);

	if (s->getEraseable()) {
		drawEraseableStroke(cr, s);
		return;
	}

	int count = 0;
	double width = s->getWidth();

	// No presure sensitivity, easy draw a line...
	if (!s->hasPressure()) {
		// Set width
		cairo_set_line_width(cr, width);

		while (points.hasNext()) {
			Point p = points.next();
			if (startPoint <= count) {
				cairo_line_to(cr, p.x, p.y);
			}
			count++;
		}

		cairo_stroke(cr);
		return;
	}

	///////////////////////////////////////////////////////
	// Presure sensitiv stroken
	///////////////////////////////////////////////////////


	Point lastPoint1(-1, -1);
	lastPoint1 = points.next();
	double lastWidth = width;

	while (points.hasNext()) {
		Point p = points.next();
		if (startPoint <= count) {
			if (lastPoint1.z != Point::NO_PRESURE) {
				width = lastPoint1.z;
			}

			cairo_set_line_width(cr, width);

			cairo_move_to(cr, lastPoint1.x, lastPoint1.y);
			cairo_line_to(cr, p.x, p.y);
			cairo_stroke(cr);
		}
		count++;
		lastPoint1 = p;
	}

	cairo_stroke(cr);

}

void DocumentView::drawText(cairo_t *cr, Text * t) {
	if (t->isInEditing()) {
		return;
	}
	applyColor(cr, t);

	TextView::drawText(cr, t);
}

void DocumentView::drawImage(cairo_t *cr, Image * i) {
	cairo_matrix_t defaultMatrix = { 0 };
	cairo_get_matrix(cr, &defaultMatrix);

	cairo_surface_t * img = i->getImage();
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

void DocumentView::drawElement(cairo_t *cr, Element * e) {
	if (e->getType() == ELEMENT_STROKE) {
		drawStroke(cr, (Stroke *) e);
	} else if (e->getType() == ELEMENT_TEXT) {
		drawText(cr, (Text *) e);
	} else if (e->getType() == ELEMENT_IMAGE) {
		drawImage(cr, (Image *) e);
	}
}

void DocumentView::drawLayer(cairo_t *cr, Layer * l) {
	ListIterator<Element *> it = l->elementIterator();

#ifdef SHOW_REPAINT_BOUNDS
	int drawed = 0;
	int notDrawed = 0;
#endif //SHOW_REPAINT_BOUNDS
	while (it.hasNext()) {
		Element * e = it.next();
		CHECK_MEMORY(e);

#ifdef SHOW_ELEMENT_BOUNDS
		cairo_set_source_rgb(cr, 1, 0, 0);
		cairo_set_line_width(cr, 1);
		cairo_rectangle(cr, e->getX(), e->getY(), e->getElementWidth(), e->getElementHeight());
		cairo_stroke(cr);
#endif // SHOW_ELEMENT_BOUNDS
		//cairo_new_path(cr);

		if (this->lX != -1) {
			if (e->intersectsArea(this->lX, this->lY, this->width, this->height)) {
				drawElement(cr, e);
#ifdef SHOW_REPAINT_BOUNDS
				drawed++;
#endif //SHOW_REPAINT_BOUNDS
			} else {
#ifdef SHOW_REPAINT_BOUNDS
				notDrawed++;
#endif //SHOW_REPAINT_BOUNDS
			}

		} else {
#ifdef SHOW_REPAINT_BOUNDS
			drawed++;
#endif //SHOW_REPAINT_BOUNDS
			drawElement(cr, e);
		}
	}

#ifdef SHOW_REPAINT_BOUNDS
	printf("DocumentView: draw %i / not draw %i\n", drawed, notDrawed);
#endif //SHOW_REPAINT_BOUNDS
}

void DocumentView::paintBackgroundImage() {
	GdkPixbuf * pixbuff = page->backgroundImage.getPixbuf();
	if (pixbuff) {
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

void DocumentView::paintBackgroundColor() {
	applyColor(cr, page->getBackgroundColor());

	cairo_rectangle(cr, 0, 0, width, height);
	cairo_fill(cr);
}

const double graphSize = 14.17;

void DocumentView::paintBackgroundGraph() {
	applyColor(cr, 0x40A0FF);

	cairo_set_line_width(cr, 0.5);

	for (double x = graphSize; x < width; x += graphSize) {
		cairo_move_to(cr, x, 0);
		cairo_line_to(cr, x, height);
	}

	for (double y = graphSize; y < height; y += graphSize) {
		cairo_move_to(cr, 0, y);
		cairo_line_to(cr, width, y);
	}

	cairo_stroke(cr);
}

const double roulingSize = 24;

void DocumentView::paintBackgroundRuled() {
	applyColor(cr, 0x40A0FF);

	cairo_set_line_width(cr, 0.5);

	for (double y = 80; y < height; y += roulingSize) {
		cairo_move_to(cr, 0, y);
		cairo_line_to(cr, width, y);
	}

	cairo_stroke(cr);
}

void DocumentView::paintBackgroundLined() {
	applyColor(cr, 0x40A0FF);

	cairo_set_line_width(cr, 0.5);

	applyColor(cr, 0xFF0080);
	cairo_move_to(cr, 72, 0);
	cairo_line_to(cr, 72, height);
	cairo_stroke(cr);
}

void DocumentView::drawSelection(cairo_t * cr, ElementContainer * container) {
	for (GList * l = container->getElements(); l != NULL; l = l->next) {
		Element * e = (Element *) l->data;
		drawElement(cr, e);
	}
}

void DocumentView::limitArea(double x, double y, double width, double heigth) {
	this->lX = x;
	this->lY = y;
	this->lWidth = width;
	this->lHeight = heigth;
}

void DocumentView::drawPage(XojPage * page, XojPopplerPage * popplerPage, cairo_t * cr, bool forPrinting) {
	this->cr = cr;
	this->page = page;
	this->width = page->getWidth();
	this->height = page->getHeight();

	CHECK_MEMORY(page);

	if (page->getBackgroundType() == BACKGROUND_TYPE_PDF) {
		if (popplerPage) {
			popplerPage->render(cr, forPrinting);

			cairo_set_operator(cr, CAIRO_OPERATOR_DEST_OVER);
			cairo_set_source_rgb(cr, 1., 1., 1.);
			cairo_paint(cr);
		} else if (!forPrinting) {
			cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
			cairo_set_font_size(cr, 26);

			cairo_set_source_rgb(cr, 0.8, 0.8, 0.8);

			cairo_text_extents_t extents = { 0 };
			const char * loading = _("PDF Background missing");

			cairo_text_extents(cr, loading, &extents);
			cairo_move_to(cr, this->width / 2 - extents.width / 2, this->height / 2 - extents.height / 2);
			cairo_show_text(cr, loading);
		}
	} else if (page->getBackgroundType() == BACKGROUND_TYPE_IMAGE) {
		paintBackgroundImage();
	} else if (page->getBackgroundType() == BACKGROUND_TYPE_GRAPH) {
		paintBackgroundColor();
		paintBackgroundGraph();
	} else if (page->getBackgroundType() == BACKGROUND_TYPE_LINED) {
		paintBackgroundColor();
		paintBackgroundRuled();
		paintBackgroundLined();
	} else if (page->getBackgroundType() == BACKGROUND_TYPE_RULED) {
		paintBackgroundColor();
		paintBackgroundRuled();
	} else if (page->getBackgroundType() == BACKGROUND_TYPE_NONE) {
		paintBackgroundColor();
	}

	cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);

	int layer = 0;
	ListIterator<Layer *> it = page->layerIterator();
	while (it.hasNext() && layer < page->getSelectedLayerId()) {
		Layer * l = it.next();
		l->isMemoryCorrupted();
		drawLayer(cr, l);
		layer++;
	}

#ifdef SHOW_REPAINT_BOUNDS
	if (this->lX != -1) {
		printf("repaint area\n");
		cairo_set_source_rgb(cr, 1, 0, 0);
		cairo_set_line_width(cr, 1);
		cairo_rectangle(cr, this->lX + 3, this->lY + 3, this->lWidth - 6, this->lHeight - 6);
		cairo_stroke(cr);
	} else {
		printf("repaint complete\n");
	}
#endif //SHOW_REPAINT_BOUNDS
	this->lX = -1;
	this->lY = -1;
	this->lWidth = -1;
	this->lHeight = -1;

	this->page = NULL;
	this->cr = NULL;
}
