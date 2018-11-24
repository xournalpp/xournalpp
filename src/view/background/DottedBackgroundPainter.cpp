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

void DottedBackgroundPainter::paint()
{
	XOJ_CHECK_TYPE(DottedBackgroundPainter);

	paintBackgroundColor();
	paintBackgroundDotted();
}

const double graphSize = 14.17;

void DottedBackgroundPainter::paintBackgroundDotted()
{
	XOJ_CHECK_TYPE(DottedBackgroundPainter);

	Util::cairo_set_source_rgbi(cr, 0xBDBDBD);

	cairo_set_line_width(cr, 0.5);

	cairo_set_line_width(cr, 1.5);
	cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);

	for (double x = graphSize; x < width; x += graphSize)
	{
		for (double y = graphSize; y < height; y += graphSize)
		{
			cairo_move_to(cr, x, y);
			cairo_line_to(cr, x, y);
		}
	}

	cairo_stroke(cr);
}
