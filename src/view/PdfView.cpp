#include "PdfView.h"
#include <config.h>
#include <glib/gi18n-lib.h>

PdfView::PdfView() {
}

PdfView::~PdfView() {
}

void PdfView::drawPage(PdfCache * cache, XojPopplerPage * popplerPage, cairo_t * cr, double zoom, double width, double height, bool forPrinting) {
	if (popplerPage) {
		if (cache && !forPrinting) {
			cache->render(cr, popplerPage, zoom);
		} else {
			popplerPage->render(cr, forPrinting);
		}

		if (!forPrinting) {
			cairo_set_operator(cr, CAIRO_OPERATOR_DEST_OVER);
			cairo_set_source_rgb(cr, 1., 1., 1.);
		}
		cairo_paint(cr);
	} else {
		cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
		cairo_set_font_size(cr, 26);

		cairo_set_source_rgb(cr, 0.8, 0.8, 0.8);

		cairo_text_extents_t extents = { 0 };
		const char * strMissing = _("PDF background missing");

		cairo_text_extents(cr, strMissing, &extents);
		cairo_move_to(cr, width / 2 - extents.width / 2, height / 2 - extents.height / 2);
		cairo_show_text(cr, strMissing);
	}
}

