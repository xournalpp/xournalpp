#include "BaseBackgroundPainter.h"

#include <Util.h>

BaseBackgroundPainter::BaseBackgroundPainter()
 : cr(NULL),
   width(0),
   height(0)
{
	XOJ_INIT_TYPE(BaseBackgroundPainter);
}

BaseBackgroundPainter::~BaseBackgroundPainter()
{
	XOJ_CHECK_TYPE(BaseBackgroundPainter);

	XOJ_RELEASE_TYPE(BaseBackgroundPainter);
}

void BaseBackgroundPainter::paint(cairo_t* cr, PageRef page, map<string, string>& config)
{
	XOJ_CHECK_TYPE(BaseBackgroundPainter);

	this->cr = cr;
	this->page = page;
	this->config = config;

	this->width = page->getWidth();
	this->height = page->getHeight();

	paint();

	this->cr = NULL;
}

void BaseBackgroundPainter::paint()
{
	XOJ_CHECK_TYPE(BaseBackgroundPainter);

	paintBackgroundColor();
}

void BaseBackgroundPainter::paintBackgroundColor()
{
	XOJ_CHECK_TYPE(BaseBackgroundPainter);

	Util::cairo_set_source_rgbi(cr, page->getBackgroundColor());

	cairo_rectangle(cr, 0, 0, width, height);
	cairo_fill(cr);
}
