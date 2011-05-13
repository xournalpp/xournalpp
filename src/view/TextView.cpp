#include "TextView.h"
#include "../model/Text.h"
#include <string.h>
#include "../pdf/poppler/XojPopplerPage.h"

TextView::TextView() {
}

TextView::~TextView() {
}

PangoLayout * TextView::initPango(cairo_t * cr, Text * t) {
	PangoLayout * layout = pango_cairo_create_layout(cr);

	// TODO LOW PRIO: add autowrap and text field size for the next xournal release (with new fileformat...)
	//pango_layout_set_wrap

	//	pango_cairo_context_set_resolution(pango_layout_get_context(layout), 10);
	//	pango_cairo_update_layout(cr, layout);


	pango_context_set_matrix(pango_layout_get_context(layout), NULL);
	updatePangoFont(layout, t);
	return layout;
}

void TextView::updatePangoFont(PangoLayout * layout, Text * t) {
	PangoFontDescription * desc = pango_font_description_from_string(t->getFont().getName().c_str());
	pango_font_description_set_absolute_size(desc, t->getFont().getSize() * PANGO_SCALE);
	//	pango_font_description_set_size(desc, t->getFont().getSize() * PANGO_SCALE);

	pango_layout_set_font_description(layout, desc);
	pango_font_description_free(desc);
}

void TextView::drawText(cairo_t * cr, Text * t) {
	cairo_save(cr);

	cairo_translate(cr, t->getX(), t->getY());

	PangoLayout * layout = initPango(cr, t);
	String str = t->getText();
	pango_layout_set_text(layout, str.c_str(), str.size());

	pango_cairo_show_layout(cr, layout);

	g_object_unref(layout);

	cairo_restore(cr);
}

GList * TextView::findText(Text * t, const char * search) {
	cairo_surface_t * surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1);
	cairo_t * cr = cairo_create(surface);

	GList * list = NULL;

	PangoLayout * layout = initPango(cr, t);
	String str = t->getText();
	pango_layout_set_text(layout, str.c_str(), str.size());

	int pos = -1;

	String text = t->getText();

	String srch = search;

	do {
		pos = text.indexOfCaseInsensitiv(srch, pos + 1);
		if (pos != -1) {
			XojPopplerRectangle * mark = new XojPopplerRectangle();
			PangoRectangle rect = { 0 };
			pango_layout_index_to_pos(layout, pos, &rect);
			mark->x1 = ((double) rect.x) / PANGO_SCALE + t->getX();
			mark->y1 = ((double) rect.y) / PANGO_SCALE + t->getY();

			pango_layout_index_to_pos(layout, pos + srch.length(), &rect);
			mark->x2 = ((double) rect.x + rect.width) / PANGO_SCALE + t->getX();
			mark->y2 = ((double) rect.y + rect.height) / PANGO_SCALE + t->getY();

			list = g_list_append(list, mark);
		}
	} while (pos != -1);

	g_object_unref(layout);
	cairo_surface_destroy(surface);
	cairo_destroy(cr);

	return list;
}

void TextView::calcSize(Text * t, double & width, double & height) {
	cairo_surface_t * surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1);
	cairo_t * cr = cairo_create(surface);


	// TODO OPTIMIZE: cache the pango layout, check the size of pango layout first
	PangoLayout * layout = initPango(cr, t);
	String str = t->getText();
	pango_layout_set_text(layout, str.c_str(), str.size());
	int w = 0;
	int h = 0;
	pango_layout_get_size(layout, &w, &h);
	width = ((double) w) / PANGO_SCALE;
	height = ((double) h) / PANGO_SCALE;
	g_object_unref(layout);

	cairo_surface_destroy(surface);
	cairo_destroy(cr);
}
