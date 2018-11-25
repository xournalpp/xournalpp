#include "LineBackgroundPainter.h"

#include <Util.h>

LineBackgroundPainter::LineBackgroundPainter(bool ruled)
 : ruled(ruled)
{
	XOJ_INIT_TYPE(LineBackgroundPainter);
}

LineBackgroundPainter::~LineBackgroundPainter()
{
	XOJ_CHECK_TYPE(LineBackgroundPainter);

	XOJ_RELEASE_TYPE(LineBackgroundPainter);
}

void LineBackgroundPainter::resetConfig()
{
	XOJ_CHECK_TYPE(LineBackgroundPainter);

	this->backgroundColor1 = 0x40A0FF;
	this->backgroundColor2 = 0xFF0080;
}

void LineBackgroundPainter::paint()
{
	XOJ_CHECK_TYPE(LineBackgroundPainter);

	paintBackgroundColor();

	paintBackgroundRuled();

	if (!ruled)
	{
		paintBackgroundLined();
	}
}

const double roulingSize = 24;

void LineBackgroundPainter::paintBackgroundRuled()
{
	XOJ_CHECK_TYPE(LineBackgroundPainter);

	Util::cairo_set_source_rgbi(cr, this->backgroundColor1);
	cairo_set_line_width(cr, 0.5);

	for (double y = 80; y < height; y += roulingSize)
	{
		cairo_move_to(cr, 0, y);
		cairo_line_to(cr, width, y);
	}

	cairo_stroke(cr);
}

void LineBackgroundPainter::paintBackgroundLined()
{
	XOJ_CHECK_TYPE(LineBackgroundPainter);

	Util::cairo_set_source_rgbi(cr, this->backgroundColor2);
	cairo_set_line_width(cr, 0.5);

	cairo_move_to(cr, 72, 0);
	cairo_line_to(cr, 72, height);
	cairo_stroke(cr);
}
