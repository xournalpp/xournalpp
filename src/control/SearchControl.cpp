#include "SearchControl.h"

#include "model/Text.h"
#include "model/Layer.h"
#include "view/TextView.h"

SearchControl::SearchControl(PageRef page, XojPdfPageSPtr pdf)
{
	XOJ_INIT_TYPE(SearchControl);

	this->page = page;
	this->pdf = pdf;
}

SearchControl::~SearchControl()
{
	XOJ_CHECK_TYPE(SearchControl);

	freeSearchResults();

	XOJ_RELEASE_TYPE(SearchControl);
}

void SearchControl::freeSearchResults()
{
	XOJ_CHECK_TYPE(SearchControl);

	this->results.clear();
}

void SearchControl::paint(cairo_t* cr, GdkRectangle* rect, double zoom, GtkColorWrapper color)
{
	XOJ_CHECK_TYPE(SearchControl);

	// set the line always the same size on display
	cairo_set_line_width(cr, 1 / zoom);

	for (XojPdfRectangle rect : this->results)
	{
		cairo_rectangle(cr, rect.x1, rect.y1, rect.x2 - rect.x1, rect.y2 - rect.y1);
		color.apply(cr);
		cairo_stroke_preserve(cr);
		color.applyWithAlpha(cr, 0.3);
		cairo_fill(cr);
	}
}

bool SearchControl::search(string text, int* occures, double* top)
{
	XOJ_CHECK_TYPE(SearchControl);

	freeSearchResults();

	if (text.empty()) return true;

	if (this->pdf)
	{
		this->results = this->pdf->findText(text);
	}

	for (Layer* l : *this->page->getLayers())
	{
		if (!this->page->isLayerVisible(l))
		{
			continue;
		}
		
		for (Element* e : *l->getElements())
		{
			if (e->getType() == ELEMENT_TEXT)
			{
				Text* t = (Text*) e;

				vector<XojPdfRectangle> textResult = TextView::findText(t, text);

				this->results.insert(this->results.end(), textResult.begin(), textResult.end());
			}
		}
	}

	if (occures)
	{
		*occures = this->results.size();
	}

	if (top)
	{
		if (this->results.size() == 0)
		{
			*top = 0;
		}
		else
		{

			XojPdfRectangle first = this->results[0];

			double min = first.y1;
			for (XojPdfRectangle rect : this->results)
			{
				min = MIN(min, rect.y1);
			}

			*top = min;
		}
	}

	return this->results.size() > 0;
}
