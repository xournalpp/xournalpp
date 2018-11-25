#include "BaseBackgroundPainter.h"

#include <Util.h>

BaseBackgroundPainter::BaseBackgroundPainter()
 : cr(NULL),
   width(0),
   height(0),
   foregroundColor1(0),
   foregroundColor2(0),
   lineWidth(0),
   drawRaster1(1),
   margin1(0),
   roundMargin(0),
   lineWidthFactor(1)
{
	XOJ_INIT_TYPE(BaseBackgroundPainter);

	resetConfig();
}

BaseBackgroundPainter::~BaseBackgroundPainter()
{
	XOJ_CHECK_TYPE(BaseBackgroundPainter);

	XOJ_RELEASE_TYPE(BaseBackgroundPainter);
}

/**
 * Set a factor to draw the lines bolder, for previews
 */
void BaseBackgroundPainter::setLineWidthFactor(double factor)
{
	XOJ_CHECK_TYPE(BaseBackgroundPainter);

	this->lineWidthFactor = factor;
}

void BaseBackgroundPainter::resetConfig()
{
	XOJ_CHECK_TYPE(BaseBackgroundPainter);
	// Overwritten from the subclasses
}

void BaseBackgroundPainter::paint(cairo_t* cr, PageRef page, BackgroundConfig* config)
{
	XOJ_CHECK_TYPE(BaseBackgroundPainter);

	this->cr = cr;
	this->page = page;
	this->config = config;

	this->width = page->getWidth();
	this->height = page->getHeight();

	this->config->loadValueHex("f1", this->foregroundColor1);
	this->config->loadValueHex("f2", this->foregroundColor2);

	this->config->loadValue("lw", this->lineWidth);
	this->config->loadValue("r1", this->drawRaster1);

	this->config->loadValue("m1", this->margin1);
	this->config->loadValue("rm", this->roundMargin);

	// If the raster is two small, we get an andless loop....
	if (drawRaster1 < 1)
	{
		drawRaster1 = 1;
	}

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
