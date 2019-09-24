#include "CircleRecognizer.h"

#include "Inertia.h"
#include "ShapeRecognizerConfig.h"
#include "model/Stroke.h"

#include <cmath>

/**
 * Create circle stroke for inertia
 */
Stroke* CircleRecognizer::makeCircleShape(Stroke* originalStroke, Inertia& inertia)
{
	int npts = (int) (2 * inertia.rad());
	if (npts < 12)
	{
		npts = 12; // min. number of points
	}

	Stroke* s = new Stroke();
	s->applyStyleFrom(originalStroke);

	for (int i = 0; i <= npts; i++)
	{
		double x = inertia.centerX() + inertia.rad() * cos((2 * M_PI * i) / npts);
		double y = inertia.centerY() + inertia.rad() * sin((2 * M_PI * i) / npts);
		s->addPoint(Point(x, y));
	}

	return s;
}

/**
 *  Test if we have a circle; inertia has been precomputed by caller
 */
double CircleRecognizer::scoreCircle(Stroke* s, Inertia& inertia)
{
	if (inertia.getMass() == 0.0)
	{
		return 0;
	}

	double sum = 0.0;
	double x0 = inertia.centerX();
	double y0 = inertia.centerY();
	double r0 = inertia.rad();

	ArrayIterator<Point> it = s->pointIterator();

	if (!it.hasNext())
	{
		return 0;
	}

	Point p1 = it.next();

	while (it.hasNext())
	{
		Point p2 = it.next();

		double dm = hypot(p2.x - p1.x, p2.y - p1.y);
		double deltar = hypot(p1.x - x0, p1.y - y0) - r0;
		sum += dm * fabs(deltar);

		p1 = p2;
	}

	return sum / (inertia.getMass() * r0);
}

Stroke* CircleRecognizer::recognize(Stroke* stroke)
{
	Inertia s;
	s.calc(stroke->getPoints(), 0, stroke->getPointCount());
	RDEBUG("Mass=%.0f, Center=(%.1f,%.1f), I=(%.0f,%.0f, %.0f), Rad=%.2f, Det=%.4f",
			s.getMass(), s.centerX(), s.centerY(), s.xx(), s.yy(), s.xy(), s.rad(), s.det());

	if (s.det() > CIRCLE_MIN_DET)
	{
		double score = CircleRecognizer::scoreCircle(stroke, s);
		RDEBUG("Circle score: %.2f", score);
		if (score < CIRCLE_MAX_SCORE)
		{
			return CircleRecognizer::makeCircleShape(stroke, s);
		}
	}

	return nullptr;
}
