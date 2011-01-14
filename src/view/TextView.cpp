#include "TextView.h"
#include "../model/Text.h"
#include <string.h>
#include <poppler.h>

TextView::TextView() {
}

TextView::~TextView() {
}

void TextView::initCairo(cairo_t *cr, Text * t) {
	XojFont & font = t->getFont();

	PangoFontDescription * desc = pango_font_description_from_string(font.getName().c_str());
	pango_font_description_set_size(desc, font.getSize());

	bool italic = pango_font_description_get_style(desc) != PANGO_STYLE_NORMAL ;
	bool bold = pango_font_description_get_weight(desc) == PANGO_WEIGHT_BOLD;

	cairo_select_font_face(cr, pango_font_description_get_family(desc), italic? CAIRO_FONT_SLANT_ITALIC
			: CAIRO_FONT_SLANT_NORMAL, bold ? CAIRO_FONT_WEIGHT_BOLD : CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(cr, font.getSize());

	pango_font_description_free(desc);


	// TODO: use http://library.gnome.org/devel/pango/stable/pango-Cairo-Rendering.html

	//	double size = font.getSize();
	//	cairo_matrix_t m;
	//	cairo_matrix_init_scale(&m, size, size);
	//
	//	printf("size: %lf, x0 = %lf, xx = %lf, xy = %lf, y0 = %lf, yx = %lf, yy = %lf\n", size, m.x0, m.xx, m.xy, m.y0, m.yx, m.yy);
	//
	//	cairo_set_font_matrix(cr, &m);
}

void TextView::drawText(cairo_t *cr, Text * t) {
	initCairo(cr, t);

	cairo_text_extents_t extents = { 0 };
	cairo_font_extents_t fe = { 0 };

	// This need to be here, why...? I don't know, the size should be calculated anyway if t->getX() is called...
	t->getElementWidth();

	double y = 3 + t->getY();
	StringTokenizer token(t->getText(), '\n');
	const char * str = token.next();

	cairo_font_extents(cr, &fe);

	// TODO: with "Hallo                                            Test           X" this went wrong on some zoom factors...
	//	printf("fe height: %lf\n", fe.height);
	// this is because cairo fonts are not always the same size on different zoom levels!!


	while (str) {
		y += fe.height - fe.descent;

		StringTokenizer tokenTab(str, '\t', true);
		const char * strTab = tokenTab.next();

		double x = 0;

		while (strTab) {
			if (strcmp(strTab, "\t") == 0) {
				int tab = x / TextView::TAB_INDEX + 1;
				x = tab * TextView::TAB_INDEX;
			} else {
				cairo_move_to(cr, t->getX() + x, y);
				cairo_show_text(cr, strTab);

				cairo_text_extents(cr, strTab, &extents);
				x += extents.x_advance;
			}

			strTab = tokenTab.next();
		}
		y += fe.height * 0.25;

		//		double x = t->getX();
		//		for (char * c = str; *c; c++) {
		//			char tmp[2] = { 0, 0 };
		//			tmp[0] = *c;
		//			cairo_text_extents_t ex = { 0 };
		//			cairo_text_extents(cr, tmp, &ex);
		//
		//			printf("->\"%s\": w:%lf h:%lf xa:%lf ya:%lf xb:%lf yb:%lf\n", tmp, ex.width, ex.height, ex.x_advance,
		//					ex.y_advance, ex.x_bearing, ex.y_bearing);
		//			cairo_rectangle(cr, x + ex.x_bearing, y + ex.y_bearing, ex.width, ex.height);
		//			x += ex.x_advance;
		//
		//			cairo_stroke(cr);
		//		}
		//		printf("--------\n\n");

		//		printf("->\"%s\"\n", str);
		str = token.next();
	}
}

GList * TextView::findText(Text * t, const char * search) {
	GList * list = NULL;
	cairo_surface_t * surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1);
	cairo_t * cr = cairo_create(surface);

	initCairo(cr, t);

	cairo_text_extents_t extents = { 0 };
	cairo_font_extents_t fe = { 0 };
	StringTokenizer token(t->getText(), '\n');
	const char * str = token.next();
	double y = 3 + t->getY();
	double x = t->getX();
	int searchlen = strlen(search);

	cairo_font_extents(cr, &fe);

	while (str) {
		String line = str;
		int pos = -1;
		y += fe.height - fe.descent;

		do {
			pos = line.indexOfCaseInsensitiv(search, pos + 1);
			if (pos != -1) {
				cairo_text_extents(cr, line.substring(0, pos).c_str(), &extents);
				PopplerRectangle * rect = g_new(PopplerRectangle, 1);
				rect->x1 = x + extents.x_advance;
				rect->y1 = y;

				//				printf("->\"%s\": w:%lf h:%lf xa:%lf ya:%lf xb:%lf yb:%lf\n", line.substring(0, pos).c_str(),
				//						extents.width, extents.height, extents.x_advance, extents.y_advance, extents.x_bearing,
				//						extents.y_bearing);

				// because of case insensitive search, TEST and test may have different sizes (depeding of the font)
				cairo_text_extents(cr, line.substring(pos, searchlen).c_str(), &extents);

				//				printf("->\"%s\": w:%lf h:%lf xa:%lf ya:%lf xb:%lf yb:%lf\n", line.substring(pos, searchlen).c_str(),
				//						extents.width, extents.height, extents.x_advance, extents.y_advance, extents.x_bearing,
				//						extents.y_bearing);

				rect->x1 += extents.x_bearing;
				rect->y1 += extents.y_bearing;
				rect->x2 = rect->x1 + extents.x_advance;
				rect->y2 = rect->y1 + extents.height;

				//				printf("found text pos: %i::: %lf %lf %lf %lf\n\n", pos, rect->x1, rect->y1, rect->x2, rect->y2);
				list = g_list_append(list, rect);
			}

		} while (pos != -1);

		str = token.next();
		y += fe.height * 0.25;
	}

	cairo_surface_destroy(surface);
	cairo_destroy(cr);

	return list;
}

void TextView::calcSize(Text * t, double & width, double & height) {
	cairo_surface_t * surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1);
	cairo_t *cr;

	cr = cairo_create(surface);

	initCairo(cr, t);

	cairo_text_extents_t extents = { 0 };
	cairo_font_extents_t fe = { 0 };
	cairo_font_extents(cr, &fe);

	height = 3;
	width = 0;
	StringTokenizer token(t->getText(), '\n');
	const char * str = token.next();

	while (str) {
		cairo_text_extents(cr, str, &extents);

		height += fe.height * 1.25 - fe.descent;

		width = MAX(width, extents.x_advance);

		str = token.next();
		if (!str) {
			height -= fe.height * 0.25;
		}
	}

	cairo_surface_destroy(surface);
	cairo_destroy(cr);
}
