#include "StrokeView.h"
#include "DocumentView.h"

#include "model/eraser/EraseableStroke.h"
#include "model/Stroke.h"

StrokeView::StrokeView(cairo_t* cr, Stroke* s, int startPoint, double scaleFactor, bool noAlpha)
 : cr(cr),
   s(s),
   startPoint(startPoint),
   scaleFactor(scaleFactor),
   noAlpha(noAlpha)
{
}

StrokeView::~StrokeView()
{
}

void StrokeView::drawFillStroke()
{
	ArrayIterator<Point> points = s->pointIterator();

	if (points.hasNext())
	{
		Point p = points.next();
		cairo_move_to(cr, p.x, p.y);
	}
	else
	{
		return;
	}

	while (points.hasNext())
	{
		Point p = points.next();
		cairo_line_to(cr, p.x, p.y);
	}

	cairo_fill(cr);
}

void StrokeView::applyDashed(double offset)
{
	const double* dashes = nullptr;
	int dashCount = 0;
	if (s->getLineStyle().getDashes(dashes, dashCount))
	{
		cairo_set_dash(cr, dashes, dashCount, offset);
	}
	else
	{
		// Disable dash
		cairo_set_dash(cr, nullptr, 0, 0);
	}
}

void StrokeView::drawEraseableStroke(cairo_t* cr, Stroke* s)
{
	EraseableStroke* e = s->getEraseable();
	e->draw(cr);
}

/**
 * Change cairo source, used to draw hilighter transparent,
 * but only if not currently drawing and so on (yes, complicated)
 */
void StrokeView::changeCairoSource(bool markAudioStroke)
{
	///////////////////////////////////////////////////////
	// Fill stroke
	///////////////////////////////////////////////////////
	if (s->getFill() != -1 && s->getToolType() != STROKE_TOOL_HIGHLIGHTER)
	{
		cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

		// Set the color and transparency
		DocumentView::applyColor(cr, s, s->getFill());

		drawFillStroke();
	}


	if (s->getToolType() == STROKE_TOOL_HIGHLIGHTER ||
		(s->getAudioFilename().length() == 0 && markAudioStroke))
	{
		if (s->getToolType() == STROKE_TOOL_HIGHLIGHTER) {
			cairo_set_operator(cr, CAIRO_OPERATOR_MULTIPLY);
		}
		else {
			cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
		}

		// Set the color
		DocumentView::applyColor(cr, s, 120);
	}
	else
	{
		cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
		// Set the color
		DocumentView::applyColor(cr, s);
	}
}

/**
 * No pressure sensitivity, one line is drawn
 */
void StrokeView::drawNoPressure()
{
	int count = 1;
	double width = s->getWidth();
	ArrayIterator<Point> points = s->pointIterator();

	bool group = false;
	if (s->getFill() != -1 && s->getToolType() == STROKE_TOOL_HIGHLIGHTER)
	{
		cairo_push_group(cr);
		// Do not apply the alpha here, else the border and the fill
		// are visible instead of one homogeneous area
		DocumentView::applyColor(cr, s, 255);
		drawFillStroke();
		group = true;
	}

	// Set width
	cairo_set_line_width(cr, width * scaleFactor);
	applyDashed(0);

	Point lastPoint = points.get();

	while (points.hasNext())
	{
		Point p = points.next();

		if (startPoint <= count)
		{
			cairo_line_to(cr, p.x, p.y);
		}
		else
		{
			cairo_move_to(cr, p.x, p.y);
		}

		count++;
		lastPoint = p;
	}

	cairo_stroke(cr);

	if (group)
	{
		cairo_pop_group_to_source(cr);

		if (noAlpha)
		{
			// Currently drawing -> transparent applied on blitting
			cairo_paint(cr);
		}
		else
		{
			cairo_paint_with_alpha(cr, s->getFill() / 255.0);
		}
	}
}

/**
 * Draw a stroke with pressure, for this multiple
 * lines with different widths needs to be drawn
 */
void StrokeView::drawWithPressuire()
{
	int count = 1;
	double width = s->getWidth();
	ArrayIterator<Point> points = s->pointIterator();

	Point lastPoint1 = points.next();
	double dashOffset = 0;
	while (points.hasNext())
	{
		Point p = points.next();
		if (startPoint <= count)
		{
			if (lastPoint1.z != Point::NO_PRESSURE)
			{
				width = lastPoint1.z;
			}

			// Set width
			cairo_set_line_width(cr, width * scaleFactor);
			applyDashed(dashOffset);

			cairo_move_to(cr, lastPoint1.x, lastPoint1.y);
			cairo_line_to(cr, p.x, p.y);
			cairo_stroke(cr);
		}
		count++;
		dashOffset += lastPoint1.lineLengthTo(p);

		lastPoint1 = p;
	}

	cairo_stroke(cr);
}

void StrokeView::paint(bool dontRenderEditingStroke)
{
	cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
	cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);

	// don't render eraseable for previews
	if (s->getEraseable() && !dontRenderEditingStroke)
	{
		drawEraseableStroke(cr, s);
		return;
	}

	// No pressure sensitivity, easy draw a line...
	if (!s->hasPressure() || s->getToolType() == STROKE_TOOL_HIGHLIGHTER)
	{
		drawNoPressure();
	}
	else
	{
		drawWithPressuire();
	}
}

