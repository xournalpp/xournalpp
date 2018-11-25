#include "GraphBackgroundPainter.h"

#include <Util.h>

GraphBackgroundPainter::GraphBackgroundPainter()
{
	XOJ_INIT_TYPE(GraphBackgroundPainter);

}

GraphBackgroundPainter::~GraphBackgroundPainter()
{
	XOJ_CHECK_TYPE(GraphBackgroundPainter);

	XOJ_RELEASE_TYPE(GraphBackgroundPainter);
}

void GraphBackgroundPainter::resetConfig()
{
	XOJ_CHECK_TYPE(GraphBackgroundPainter);

	this->foregroundColor1 = 0xBDBDBD;
	this->lineWidth = 0.5;
}

void GraphBackgroundPainter::paint()
{
	XOJ_CHECK_TYPE(GraphBackgroundPainter);

	paintBackgroundColor();
	paintBackgroundGraph();
}

const double graphSize = 14.17;

void GraphBackgroundPainter::paintBackgroundGraph()
{
	XOJ_CHECK_TYPE(GraphBackgroundPainter);

	Util::cairo_set_source_rgbi(cr, this->foregroundColor1);

	cairo_set_line_width(cr, lineWidth * lineWidthFactor);

	for (double x = graphSize; x < width; x += graphSize)
	{
		cairo_move_to(cr, x, 0);
		cairo_line_to(cr, x, height);
	}

	for (double y = graphSize; y < height; y += graphSize)
	{
		cairo_move_to(cr, 0, y);
		cairo_line_to(cr, width, y);
	}

	cairo_stroke(cr);
}
