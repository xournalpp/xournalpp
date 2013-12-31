#include "XojPopplerPage.h"
#include <poppler/TextOutputDev.h>
#include <poppler/PDFDoc.h>
#include <poppler/Gfx.h>
#include <poppler/OutputDev.h>
#include "../workaround/workaround.h"

XojPopplerPage::XojPopplerPage(PDFDoc* doc, GMutex* docMutex,
                               CairoOutputDev* outputDev, Page* page, int index)
{
	XOJ_INIT_TYPE(XojPopplerPage);

	this->doc = doc;
	this->page = page;
	this->index = index;

	this->outputDev = outputDev;
	this->text = NULL;

	this->renderMutex = g_mutex_new();
	this->docMutex = docMutex;
}

XojPopplerPage::~XojPopplerPage()
{
	XOJ_CHECK_TYPE(XojPopplerPage);

	if (this->text)
	{
		this->text->decRefCnt();
	}

	g_mutex_free(this->renderMutex);
	this->renderMutex = NULL;

	this->docMutex = NULL;

	XOJ_RELEASE_TYPE(XojPopplerPage);
}

double XojPopplerPage::getWidth()
{
	XOJ_CHECK_TYPE(XojPopplerPage);

	int rotate = this->page->getRotate();
	if (rotate == 90 || rotate == 270)
	{
		return this->page->getCropHeight();
	}
	else
	{
		return this->page->getCropWidth();
	}
}

double XojPopplerPage::getHeight()
{
	XOJ_CHECK_TYPE(XojPopplerPage);

	int rotate = this->page->getRotate();
	if (rotate == 90 || rotate == 270)
	{
		return this->page->getCropWidth();
	}
	else
	{
		return this->page->getCropHeight();
	}
}

static GBool poppler_print_annot_cb(Annot* annot, void* user_data)
{
	if (annot->getFlags() & Annot::flagPrint)
	{
		return true;
	}
	return (annot->getType() == Annot::typeWidget);
}

void XojPopplerPage::render(cairo_t* cr, bool forPrinting)
{
	XOJ_CHECK_TYPE(XojPopplerPage);

	g_mutex_lock(this->renderMutex);

	this->outputDev->setCairo(cr);
	this->outputDev->setPrinting(forPrinting);

	if (!forPrinting)
	{
		if (!this->text)
		{
			this->text = new TextPage(false);
		}

		this->outputDev->setTextPage(this->text);
	}

	/* NOTE: instead of passing -1 we should/could use cairo_clip_extents()
	 * to get a bounding box */
	cairo_save(cr);

	g_mutex_lock(this->docMutex);

	this->page->displaySlice(this->outputDev, 72.0, 72.0, 0,
	                         false, /* useMediaBox */
	                         true, /* Crop */
	                         -1, -1, -1, -1, forPrinting, this->doc->getCatalog(), NULL, NULL,
	                         forPrinting ? poppler_print_annot_cb : NULL, NULL);
	cairo_restore(cr);

	g_mutex_unlock(this->docMutex);

	this->outputDev->setCairo(NULL);
	this->outputDev->setTextPage(NULL);

	g_mutex_unlock(this->renderMutex);
}

void XojPopplerPage::initTextPage()
{
	XOJ_CHECK_TYPE(XojPopplerPage);

	g_mutex_lock(this->renderMutex);

	if (this->text == NULL)
	{
		g_mutex_lock(this->docMutex);
		TextOutputDev* textDev = new TextOutputDev(NULL, true, false, false);
		Gfx* gfx = this->page->createGfx(textDev, 72.0, 72.0, 0,
		                                 false, /* useMediaBox */
		                                 true, /* Crop */
		                                 -1, -1, -1, -1, false, /* printing */
		                                 this->doc->getCatalog(), NULL, NULL, NULL, NULL);
		this->page->display(gfx);
		textDev->endPage();

		this->text = textDev->takeText();
		delete gfx;
		delete textDev;
		g_mutex_unlock(this->docMutex);
	}

	g_mutex_unlock(this->renderMutex);
}

Page* XojPopplerPage::getPage()
{
	XOJ_CHECK_TYPE(XojPopplerPage);

	return this->page;
}

GList* XojPopplerPage::findText(const char* text)
{
	XOJ_CHECK_TYPE(XojPopplerPage);

	XojPopplerRectangle* match;
	GList* matches;
	double xMin, yMin, xMax, yMax;
	gunichar* ucs4;
	glong ucs4_len;

	initTextPage();

	ucs4 = g_utf8_to_ucs4_fast(text, -1, &ucs4_len);

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

	while (this->text->findText(ucs4, ucs4_len, atTop,
	                            true, // startAtTop, stopAtBottom
	                            !atTop, false, // startAtLast, stopAtLast
	                            false, false, // caseSensitive, backwards
	                            &xMin, &yMin, &xMax, &yMax))
	{

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

XojPopplerRectangle::XojPopplerRectangle()
{
	this->x1 = -1;
	this->x2 = -1;
	this->y1 = -1;
	this->y2 = -1;
}

