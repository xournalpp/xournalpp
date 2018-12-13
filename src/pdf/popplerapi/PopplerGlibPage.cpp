#include "PopplerGlibPage.h"


PopplerGlibPage::PopplerGlibPage(PopplerPage* page)
 : page(page)
{
	XOJ_INIT_TYPE(PopplerGlibPage);
}

PopplerGlibPage::~PopplerGlibPage()
{
	XOJ_CHECK_TYPE(PopplerGlibPage);

	page = NULL;

	XOJ_RELEASE_TYPE(PopplerGlibPage);
}

double PopplerGlibPage::getWidth()
{
	XOJ_CHECK_TYPE(PopplerGlibPage);

	double width = 0;
	poppler_page_get_size(page, &width, NULL);

	return width;
}

double PopplerGlibPage::getHeight()
{
	XOJ_CHECK_TYPE(PopplerGlibPage);

	double height = 0;
	poppler_page_get_size(page, NULL, &height);

	return height;
}

void PopplerGlibPage::render(cairo_t* cr, bool forPrinting)
{
	XOJ_CHECK_TYPE(PopplerGlibPage);

	if (forPrinting)
	{
		poppler_page_render_for_printing(page, cr);
	}
	else
	{
		poppler_page_render(page, cr);
	}
}

vector<XojPdfRectangle> PopplerGlibPage::findText(string& text)
{
	XOJ_CHECK_TYPE(PopplerGlibPage);

	vector<XojPdfRectangle> findings;

	double height = getHeight();
	GList* matches = poppler_page_find_text(page, text.c_str());

	for (GList* l = matches; l && l->data; l = g_list_next(l))
	{
		PopplerRectangle* rect = (PopplerRectangle*) l->data;

		findings.push_back(XojPdfRectangle(rect->x1, height - rect->y1, rect->x2, height - rect->y2));

		poppler_rectangle_free(rect);
	}
	g_list_free(matches);

	return findings;
}
