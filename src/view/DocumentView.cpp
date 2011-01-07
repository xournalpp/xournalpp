#include "DocumentView.h"
#include "TextView.h"
#include "../gettext.h"
#include <gdk/gdk.h>

//#define SHOW_ELEMENT_BOUNDS


DocumentView::DocumentView() {
	this->page = NULL;
	this->cr = NULL;
}

DocumentView::~DocumentView() {
}

void DocumentView::applyColor(cairo_t *cr, Element * e, int alpha) {
	applyColor(cr, e->getColor(), alpha);
}

void DocumentView::applyColor(cairo_t *cr, int c, int alpha) {
	double r = ((c >> 16) & 0xff) / 255.0;
	double g = ((c >> 8) & 0xff) / 255.0;
	double b = (c & 0xff) / 255.0;

	cairo_set_source_rgba(cr, r, g, b, alpha / 255.0);
}

void DocumentView::drawStroke(cairo_t *cr, Stroke * s, int startPoint) {
	ArrayIterator<Point> points = s->pointIterator();
	ArrayIterator<double> widths = s->widthIterator();

	if (!points.hasNext()) {
		// Empty stroke... Should not happen
		return;
	}

	if (s->getToolType() == STROKE_TOOL_HIGHLIGHTER) {
		cairo_set_operator(cr, CAIRO_OPERATOR_ATOP);
		// Set the color
		applyColor(cr, s, 120);
	} else {
		cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
		// Set the color
		applyColor(cr, s);
	}

	cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
	cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);

	int count = 0;
	double width = s->getWidth();

	// No presure sensitivity, easy draw a line...
	if (!widths.hasNext()) {
		// Set width
		cairo_set_line_width(cr, width);

		if (s->isMoved()) { // in moving, special case
			double dx = s->getDx();
			double dy = s->getDy();

			while (points.hasNext()) {
				Point p = points.next();
				if (startPoint <= count) {
					cairo_line_to(cr, p.x + dx, p.y + dy);
				}
				count++;
			}
		} else {
			while (points.hasNext()) {
				Point p = points.next();
				if (startPoint <= count) {
					cairo_line_to(cr, p.x, p.y);
				}
				count++;
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
	double lastWidth = width;

	if (s->isMoved()) { // in moving, special case
		double dx = s->getDx();
		double dy = s->getDy();

		while (points.hasNext()) {
			Point p = points.next();
			if (widths.hasNext()) {
				width = widths.next();
			}
			if (startPoint <= count) {
				cairo_set_line_width(cr, width);

				cairo_move_to(cr, lastPoint1.x + dx, lastPoint1.y + dy);
				cairo_line_to(cr, p.x + dx, p.y + dy);
				cairo_stroke(cr);
			}
			count++;
			lastPoint1 = p;
		}
	} else {
		while (points.hasNext()) {
			Point p = points.next();
			if (widths.hasNext()) {
				width = widths.next();
			}
			if (startPoint <= count) {
				cairo_set_line_width(cr, width);

				cairo_move_to(cr, lastPoint1.x, lastPoint1.y);
				cairo_line_to(cr, p.x, p.y);
				cairo_stroke(cr);
			}
			count++;
			lastPoint1 = p;
		}
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

	cairo_set_source_surface(cr, img, i->getX()/xFactor, i->getY()/yFactor);
	cairo_paint(cr);

	cairo_set_matrix(cr, &defaultMatrix);
}

void DocumentView::drawLayer(cairo_t *cr, Layer * l) {
	printf("draw layer\n");

	ListIterator<Element *> it = l->elementIterator();
	while (it.hasNext()) {
		Element * e = it.next();
		e->debugTestIsOk();

#ifdef SHOW_ELEMENT_BOUNDS
		cairo_set_source_rgb(cr, 1, 0, 0);
		cairo_set_line_width(cr, 1);
		cairo_rectangle(cr, e->getX(), e->getY(), e->getElementWidth(), e->getElementHeight());
		cairo_stroke(cr);
#endif // SHOW_ELEMENT_BOUNDS
		cairo_new_path(cr);

		if (e->getType() == ELEMENT_STROKE) {
			drawStroke(cr, (Stroke *) e);
		} else if (e->getType() == ELEMENT_TEXT) {
			drawText(cr, (Text *) e);
		} else if (e->getType() == ELEMENT_IMAGE) {
			drawImage(cr, (Image *) e);
		}
	}

	printf("draw layer end\n");
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

void DocumentView::paintBackgroundLined() {
	applyColor(cr, 0x40A0FF);

	cairo_set_line_width(cr, 0.5);

	for (double y = 80; y < height; y += roulingSize) {
		cairo_move_to(cr, 0, y);
		cairo_line_to(cr, width, y);
	}

	cairo_stroke(cr);
}

void DocumentView::paintBackgroundRuled() {
	applyColor(cr, 0x40A0FF);

	cairo_set_line_width(cr, 0.5);

	applyColor(cr, 0xFF0080);
	cairo_move_to(cr, 72, 0);
	cairo_line_to(cr, 72, height);
	cairo_stroke(cr);
}

void DocumentView::drawPage(XojPage * page, PopplerPage * popplerPage, cairo_t *cr) {
	this->cr = cr;
	this->page = page;
	this->width = page->getWidth();
	this->height = page->getHeight();

	page->debugTestIsOk();

	if (page->getBackgroundType() == BACKGROUND_TYPE_PDF) {
		if (popplerPage) {
			//			cairo_rotate (cr, rc->rotation * G_PI / 180.0);

			poppler_page_render(popplerPage, cr);
			cairo_set_operator(cr, CAIRO_OPERATOR_DEST_OVER);
			cairo_set_source_rgb(cr, 1., 1., 1.);
			cairo_paint(cr);
		} else {
			cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
			cairo_set_font_size(cr, 26);

			cairo_set_source_rgb(cr, 0.8, 0.8, 0.8);

			cairo_text_extents_t extents = { 0 };
			const char * loading = _("Loading...");

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
		l->debugTestIsOk();
		drawLayer(cr, l);
		layer++;
	}

	this->page = NULL;
	this->cr = NULL;
}
