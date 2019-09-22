#include "GraphBackgroundPainter.h"

#include <Util.h>
#include <cmath>

GraphBackgroundPainter::GraphBackgroundPainter()
{
}

GraphBackgroundPainter::~GraphBackgroundPainter()
{
}

void GraphBackgroundPainter::resetConfig()
{
	this->foregroundColor1 = 0xBDBDBD;
	this->lineWidth = 0.5;
	this->drawRaster1 = 14.17;
	this->margin1 = 0;
	this->roundMargin = 0;
}

double GraphBackgroundPainter::getUnitSize()
{
	return this->drawRaster1;
}

void GraphBackgroundPainter::paint()
{
	paintBackgroundColor();
	paintBackgroundGraph();
}

void GraphBackgroundPainter::paintBackgroundGraph()
{
	Util::cairo_set_source_rgbi(cr, this->foregroundColor1);

	cairo_set_line_width(cr, lineWidth * lineWidthFactor);
	double marginTopBottom = margin1;
	double marginLeftRight = margin1;
	double startX = drawRaster1;
	double startY = drawRaster1;
	double snappingOffset = 2.5;

	if (roundMargin)
	{
		double w = width - 2 * marginLeftRight;
		double r = w - floor(w / drawRaster1) * drawRaster1;
		marginLeftRight += r / 2;
		//startX = marginLeftRight;

		double h = height - 2 * marginTopBottom;
		r = h - floor(h / drawRaster1) * drawRaster1;
		marginTopBottom += r / 2;
		//startY = marginTopBottom;
	}

	for (double x = startX; x < width; x += drawRaster1)
	{
		if (x < margin1 || x > (width - margin1))
		{
			continue;
		}
		cairo_move_to(cr, x, marginTopBottom-snappingOffset);
		cairo_line_to(cr, x, height - marginTopBottom-snappingOffset);
	}

	for (double y = startY; y < height; y += drawRaster1)
	{
		if (y < margin1 || y > (height - marginTopBottom))
		{
			continue;
		}

		cairo_move_to(cr, marginLeftRight, y);
		cairo_line_to(cr, width - marginLeftRight, y);
	}

	cairo_stroke(cr);
}
