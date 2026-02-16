#include "PdfCache.h"

#include <string>  // for string

#include <glib.h>  // for g_warning

#include "util/i18n.h"  // for _

void xoj::view::PdfCache::renderMissingPdfPage(cairo_t* cr, size_t pdfPageNo, double pageWidth, double pageHeight) {
    g_warning("PdfCache: Could not get the pdf page %zu from the document", pdfPageNo);

    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 26);

    cairo_set_source_rgb(cr, 0.8, 0.8, 0.8);

    cairo_text_extents_t extents = {0};
    std::string strMissing = _("PDF background missing");

    cairo_text_extents(cr, strMissing.c_str(), &extents);
    cairo_move_to(cr, pageWidth / 2 - extents.width / 2, pageHeight / 2 - extents.height / 2);
    cairo_text_path(cr, strMissing.c_str());
}
