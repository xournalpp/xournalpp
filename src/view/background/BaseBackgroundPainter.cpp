#include "BaseBackgroundPainter.h"

#include <Util.h>

BaseBackgroundPainter::BaseBackgroundPainter()
 : cr(NULL),
   width(0),
   height(0),
   backgroundColor1(0),
   backgroundColor2(0)
{
	XOJ_INIT_TYPE(BaseBackgroundPainter);

	resetConfig();
}

BaseBackgroundPainter::~BaseBackgroundPainter()
{
	XOJ_CHECK_TYPE(BaseBackgroundPainter);

	XOJ_RELEASE_TYPE(BaseBackgroundPainter);
}

void BaseBackgroundPainter::resetConfig()
{
	XOJ_CHECK_TYPE(BaseBackgroundPainter);
}

void BaseBackgroundPainter::paint(cairo_t* cr, PageRef page, BackgroundConfig* config)
{
	XOJ_CHECK_TYPE(BaseBackgroundPainter);

	this->cr = cr;
	this->page = page;
	this->config = config;

	this->width = page->getWidth();
	this->height = page->getHeight();

	this->config->loadValueHex("b1", this->backgroundColor1);
	this->config->loadValueHex("b2", this->backgroundColor2);

	paint();

	this->cr = NULL;
	this->config = NULL;
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
