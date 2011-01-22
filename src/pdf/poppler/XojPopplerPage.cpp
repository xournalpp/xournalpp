#include "XojPopplerPage.h"
#include <poppler/TextOutputDev.h>
#include <poppler/CairoOutputDev.h>
#include <poppler/PDFDoc.h>
#include <poppler/Gfx.h>

XojPopplerPage::XojPopplerPage(PDFDoc * doc, CairoOutputDev * outputDev, Page * page, int index) {
	this->doc = doc;
	this->page = page;
	this->index = index;

	this->outputDev = outputDev;
	this->text = NULL;
}

XojPopplerPage::~XojPopplerPage() {
	if (this->text) {
		this->text->decRefCnt();
	}
}

double XojPopplerPage::getWidth() {
	int rotate = this->page->getRotate();
	if (rotate == 90 || rotate == 270) {
		return this->page->getCropHeight();
	} else {
		return this->page->getCropWidth();
	}
}

double XojPopplerPage::getHeight() {
	int rotate = this->page->getRotate();
	if (rotate == 90 || rotate == 270) {
		return this->page->getCropWidth();
	} else {
		return this->page->getCropHeight();
	}
}

static GBool poppler_print_annot_cb(Annot *annot, void *user_data) {
	if (annot->getFlags() & Annot::flagPrint)
		return gTrue;
	return (annot->getType() == Annot::typeWidget);
}

void XojPopplerPage::render(cairo_t * cr, bool forPrinting) {
	this->outputDev->setCairo(cr);
	this->outputDev->setPrinting(forPrinting);

	if (!forPrinting) {
		if (!this->text) {
			this->text = new TextPage(false);
		}

		this->outputDev->setTextPage(this->text);
	}

	/* NOTE: instead of passing -1 we should/could use cairo_clip_extents()
	 * to get a bounding box */
	cairo_save(cr);
	this->page->displaySlice(this->outputDev, 72.0, 72.0, 0, false, /* useMediaBox */
	true, /* Crop */
	-1, -1, -1, -1, forPrinting, this->doc->getCatalog(), NULL, NULL, forPrinting ? poppler_print_annot_cb : NULL, NULL);
	cairo_restore(cr);

	this->outputDev->setCairo(NULL);
	this->outputDev->setTextPage(NULL);
}

void XojPopplerPage::initTextPage() {
	if (this->text == NULL) {
		TextOutputDev *text_dev = new TextOutputDev(NULL, true, false, false);
		Gfx *gfx = this->page->createGfx(text_dev, 72.0, 72.0, 0, false, /* useMediaBox */
		true, /* Crop */
		-1, -1, -1, -1, false, /* printing */
		this->doc->getCatalog(), NULL, NULL, NULL, NULL);
		this->page->display(gfx);
		text_dev->endPage();

		this->text = text_dev->takeText();
		delete gfx;
		delete text_dev;
	}
}

Page * XojPopplerPage::getPage() {
	return this->page;
}

GList * XojPopplerPage::findText(const char * text) {
	XojPopplerRectangle *match;
	GList *matches;
	double xMin, yMin, xMax, yMax;
	gunichar *ucs4;
	glong ucs4_len;

	initTextPage();

	ucs4 = g_utf8_to_ucs4_fast(text, -1, &ucs4_len);
	double height = getHeight();

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

	while (this->text->findText(ucs4, ucs4_len, atTop, true, // startAtTop, stopAtBottom
			!atTop, false, // startAtLast, stopAtLast
			false, false, // caseSensitive, backwards
			&xMin, &yMin, &xMax, &yMax)) {

		match = new XojPopplerRectangle();
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

XojPopplerRectangle::XojPopplerRectangle() {
	this->x1 = -1;
	this->x2 = -1;
	this->y1 = -1;
	this->y2 = -1;
}

