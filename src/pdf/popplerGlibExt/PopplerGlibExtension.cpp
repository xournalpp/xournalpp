#include "PopplerGlibExtension.h"

#include <poppler/Gfx.h>
#include <poppler/Page.h>
#include <poppler/PDFDoc.h>
#include <poppler/CairoOutputDev.h>

/**
 * This code is copyied from poppler-page.cc, poppler-private.h
 * This is needed to get acces to the underlying structure (a "Workaround"
 * we can also write our own poppler document hanlding istead of using glib
 */
struct _PopplerDocument {
	GObject parent_instance;
	PDFDoc *doc;

	GList *layers;
	GList *layers_rbgroups;
	CairoOutputDev *output_dev;
};

struct _PopplerPage {
	GObject parent_instance;
	PopplerDocument *document;
	Page *page;
	int index;
	TextPage *text;
	void *annots;
};

TextPage * poppler_page_get_text_page(PopplerPage *page) {
	if (page->text == NULL) {
		TextOutputDev *text_dev;
		Gfx *gfx;

		text_dev = new TextOutputDev(NULL, gTrue, gFalse, gFalse);
		gfx = page->page->createGfx(text_dev, 72.0, 72.0, 0, gFalse, /* useMediaBox */
		gTrue, /* Crop */
		-1, -1, -1, -1, gFalse, /* printing */
		page->document->doc->getCatalog(), NULL, NULL, NULL, NULL);
		page->page->display(gfx);
		text_dev->endPage();

		page->text = text_dev->takeText();
		delete gfx;
		delete text_dev;
	}

	return page->text;
}

/**
 * poppler_page_find_text:
 * @page: a #PopplerPage
 * @text: the text to search for (UTF-8 encoded)
 *
 * A #GList of rectangles for each occurance of the text on the page.
 * The coordinates are in _XOURNAL_ points.
 *
 * Return value: a #GList of PopplerRectangle,
 **/
GList *poppler_page_find_text_xoj(PopplerPage *page, const char *text) {
	PopplerRectangle *match;
	GList *matches;
	double xMin, yMin, xMax, yMax;
	gunichar *ucs4;
	glong ucs4_len;
	double height;
	TextPage * textPage;

	g_return_val_if_fail(POPPLER_IS_PAGE(page), FALSE);
	g_return_val_if_fail(text != NULL, FALSE);

	textPage = poppler_page_get_text_page(page);

	ucs4 = g_utf8_to_ucs4_fast(text, -1, &ucs4_len);
	poppler_page_get_size(page, NULL, &height);

	matches = NULL;
	xMin = 0;
	yMin = 0;

	//	  // Find a string.  If <startAtTop> is true, starts looking at the
	//	  // top of the page; else if <startAtLast> is true, starts looking
	//	  // immediately after the last find result; else starts looking at
	//	  // <xMin>,<yMin>.  If <stopAtBottom> is true, stops looking at the
	//	  // bottom of the page; else if <stopAtLast> is true, stops looking
	//	  // just before the last find result; else stops looking at
	//	  // <xMax>,<yMax>.
	//	  GBool findText(Unicode *s, int len,
	//			 GBool startAtTop, GBool stopAtBottom,
	//			 GBool startAtLast, GBool stopAtLast,
	//			 GBool caseSensitive, GBool backward,
	//			 double *xMin, double *yMin,
	//			 double *xMax, double *yMax);


	bool atTop = true;

	while (textPage->findText(ucs4, ucs4_len, atTop, true, // startAtTop, stopAtBottom
			!atTop, false, // startAtLast, stopAtLast
			false, false, // caseSensitive, backwards
			&xMin, &yMin, &xMax, &yMax)) {

		match = g_new(PopplerRectangle, 1);
		match->x1 = xMin;
		match->y1 = yMax;
		match->x2 = xMax;
		match->y2 = yMin;
		matches = g_list_prepend(matches, match);
		atTop = false;
	}

	g_free(ucs4);

	return g_list_reverse(matches);
}
