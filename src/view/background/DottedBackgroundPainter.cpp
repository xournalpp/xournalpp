#include "DottedBackgroundPainter.h"

#include <Util.h>

DottedBackgroundPainter::DottedBackgroundPainter()
{
	XOJ_INIT_TYPE(DottedBackgroundPainter);

}

DottedBackgroundPainter::~DottedBackgroundPainter()
{
	XOJ_CHECK_TYPE(DottedBackgroundPainter);

	XOJ_RELEASE_TYPE(DottedBackgroundPainter);
}

void DottedBackgroundPainter::resetConfig()
{
	XOJ_CHECK_TYPE(DottedBackgroundPainter);

	this->foregroundColor1 = 0xBDBDBD;
	this->lineWidth = 1.5;
	this->drawRaster1 = 14.17;
}

void DottedBackgroundPainter::paint()
{
	XOJ_CHECK_TYPE(DottedBackgroundPainter);

	paintBackgroundColor();
	paintBackgroundDotted();
}

void DottedBackgroundPainter::paintBackgroundDotted()
{
	XOJ_CHECK_TYPE(DottedBackgroundPainter);

	Util::cairo_set_source_rgbi(cr, this->foregroundColor1);

	cairo_set_line_width(cr, lineWidth * lineWidthFactor);
	cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);

	for (double x = drawRaster1; x < width; x += drawRaster1)
	{
		for (double y = drawRaster1; y < height; y += drawRaster1)
		{
			cairo_move_to(cr, x, y);
			cairo_line_to(cr, x, y);
		}
	}

	cairo_stroke(cr);
}
