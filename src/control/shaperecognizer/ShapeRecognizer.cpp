#include "ShapeRecognizer.h"

#include "CircleRecognizer.h"
#include "Inertia.h"
#include "ShapeRecognizerResult.h"

#include "model/Stroke.h"

#include <config-debug.h>

#include <cmath>

ShapeRecognizer::ShapeRecognizer()
{
	resetRecognizer();
	this->stroke = nullptr;
	this->queueLength = 0;
}

ShapeRecognizer::~ShapeRecognizer()
{
	resetRecognizer();
}

void ShapeRecognizer::resetRecognizer()
{
	RDEBUG("reset");

	for (int i = 0; i < MAX_POLYGON_SIDES + 1; i++)
	{
		this->queue[i].stroke = nullptr;
	}

	this->queueLength = 0;
}

/**
 *  Test if segments form standard shapes
 */
Stroke* ShapeRecognizer::tryRectangle()
{
	// first, we need whole strokes to combine to 4 segments...
	if (this->queueLength < 4)
	{
		return nullptr;
	}

	RecoSegment* rs = &this->queue[this->queueLength - 4];
	if (rs->startpt != 0)
	{
		return nullptr;
	}

	// check edges make angles ~= Pi/2 and vertices roughly match
	double avgAngle = 0.;
	for (int i = 0; i <= 3; i++)
	{
		RecoSegment* r1 = &rs[i];
		RecoSegment* r2 = &rs[(i + 1) % 4];
		if (fabs(fabs(r1->angle - r2->angle) - M_PI / 2) > RECTANGLE_ANGLE_TOLERANCE)
		{
			return nullptr;
		}
		avgAngle += r1->angle;
		if (r2->angle > r1->angle)
		{
			avgAngle += (i + 1) * M_PI / 2;
		}
		else
		{
			avgAngle -= (i + 1) * M_PI / 2;
		}

		// test if r1 points away from r2 rather than towards it
		r1->reversed = ((r1->x2 - r1->x1) * (r2->xcenter - r1->xcenter) + (r1->y2 - r1->y1) * (r2->ycenter - r1->ycenter)) < 0;
	}
	for (int i = 0; i <= 3; i++)
	{
		RecoSegment* r1 = &rs[i];
		RecoSegment* r2 = &rs[(i + 1) % 4];
		double dist = hypot((r1->reversed ? r1->x1 : r1->x2) - (r2->reversed ? r2->x2 : r2->x1),
							(r1->reversed ? r1->y1 : r1->y2) - (r2->reversed ? r2->y2 : r2->y1));
		if (dist > RECTANGLE_LINEAR_TOLERANCE * (r1->radius + r2->radius))
		{
			return nullptr;
		}
	}

	// make a rectangle of the correct size and slope
	avgAngle = avgAngle / 4;
	if (fabs(avgAngle) < SLANT_TOLERANCE)
	{
		avgAngle = 0.;
	}

	if (fabs(avgAngle) > M_PI / 2 - SLANT_TOLERANCE)
	{
		avgAngle = M_PI / 2;
	}

	Stroke* s = new Stroke();
	s->applyStyleFrom(this->stroke);

	for (int i = 0; i <= 3; i++)
	{
		rs[i].angle = avgAngle + i * M_PI / 2;
	}

	for (int i = 0; i <= 3; i++)
	{
		Point p = rs[i].calcEdgeIsect(&rs[(i + 1) % 4]);
		s->addPoint(p);
	}

	s->addPoint(s->getPoint(0));

	return s;
}

Stroke* ShapeRecognizer::tryArrow()
{
	bool rev[3];

	// first, we need whole strokes to combine to nsides segments...
	if (queueLength < 3)
	{
		return nullptr;
	}

	RecoSegment* rs = &this->queue[queueLength - 3];
	if (rs->startpt != 0)
	{
		return nullptr;
	}

	// check arrow head not too big, and orient main segment
	for (int i = 1; i <= 2; i++)
	{
		if (rs[i].radius > ARROW_MAXSIZE * rs[0].radius)
		{
			return nullptr;
		}

		rev[i] = hypot(rs[i].xcenter - rs->x1, rs[i].ycenter - rs->y1) < hypot(rs[i].xcenter - rs->x2, rs[i].ycenter - rs->y2);
	}

	if (rev[1] != rev[2])
	{
		return nullptr;
	}

	double x1;
	double y1;
	double x2;
	double y2;
	double angle;

	if (rev[1])
	{
		x1 = rs->x2;
		y1 = rs->y2;
		x2 = rs->x1;
		y2 = rs->y1;
		angle = rs->angle + M_PI;
	}
	else
	{
		x1 = rs->x1;
		y1 = rs->y1;
		x2 = rs->x2;
		y2 = rs->y2;
		angle = rs->angle;
	}

	double alpha[3];
	// check arrow head not too big, and angles roughly ok
	for (int i = 1; i <= 2; i++)
	{
		rs[i].reversed = false;
		alpha[i] = rs[i].angle - angle;
		while (alpha[i] < -M_PI / 2)
		{
			alpha[i] += M_PI;
			rs[i].reversed = !rs[i].reversed;
		}
		while (alpha[i] > M_PI / 2)
		{
			alpha[i] -= M_PI;
			rs[i].reversed = !rs[i].reversed;
		}
		RDEBUG("arrow: alpha[%d] = %.1f degrees", i, (alpha[i] * 180 / M_PI));
		if (fabs(alpha[i]) < ARROW_ANGLE_MIN || fabs(alpha[i]) > ARROW_ANGLE_MAX)
		{
			return nullptr;
		}
	}

	// check arrow head segments are roughly symmetric
	if (alpha[1] * alpha[2] > 0 || fabs(alpha[1] + alpha[2]) > ARROW_ASYMMETRY_MAX_ANGLE)
	{
		return nullptr;
	}

	if (rs[1].radius / rs[2].radius > 1 + ARROW_ASYMMETRY_MAX_LINEAR)
	{
		return nullptr;
	}

	if (rs[2].radius / rs[1].radius > 1 + ARROW_ASYMMETRY_MAX_LINEAR)
	{
		return nullptr;
	}

	// check vertices roughly match
	Point pt = rs[1].calcEdgeIsect(&rs[2]);
	for (int j = 1; j <= 2; j++)
	{
		double dist = hypot(pt.x - (rs[j].reversed ? rs[j].x1 : rs[j].x2),
							pt.y - (rs[j].reversed ? rs[j].y1 : rs[j].y2));
		RDEBUG("linear tolerance: tip[%d] = %.2f", j, (dist / rs[j].radius));
		if (dist > ARROW_TIP_LINEAR_TOLERANCE * rs[j].radius)
		{
			return nullptr;
		}
	}

	double dist = (pt.x - x2) * sin(angle) - (pt.y - y2) * cos(angle);
	dist /= rs[1].radius + rs[2].radius;

	RDEBUG("sideways gap tolerance = %.2f", dist);

	if (fabs(dist) > ARROW_SIDEWAYS_GAP_TOLERANCE)
	{
		return nullptr;
	}

	dist = (pt.x - x2) * cos(angle) + (pt.y - y2) * sin(angle);
	dist /= rs[1].radius + rs[2].radius;

	RDEBUG("main linear gap = %.2f", dist);

	if (dist < ARROW_MAIN_LINEAR_GAP_MIN || dist > ARROW_MAIN_LINEAR_GAP_MAX)
	{
		return nullptr;
	}

	// make an arrow of the correct size and slope
	if (fabs(rs->angle) < SLANT_TOLERANCE) // nearly horizontal
	{
		angle = angle - rs->angle;
		y1 = y2 = rs->ycenter;
	}

	if (rs->angle > M_PI / 2 - SLANT_TOLERANCE) // nearly vertical
	{
		angle = angle - (rs->angle - M_PI / 2);
		x1 = x2 = rs->xcenter;
	}

	if (rs->angle < -M_PI / 2 + SLANT_TOLERANCE) // nearly vertical
	{
		angle = angle - (rs->angle + M_PI / 2);
		x1 = x2 = rs->xcenter;
	}

	double delta = fabs(alpha[1] - alpha[2]) / 2;
	dist = (hypot(rs[1].x1 - rs[1].x2, rs[1].y1 - rs[1].y2) + hypot(rs[2].x1 - rs[2].x2, rs[2].y1 - rs[2].y2)) / 2;

	Stroke* s = new Stroke();
	s->applyStyleFrom(this->stroke);

	s->addPoint(Point(x1, y1));
	s->addPoint(Point(x2, y2));

	s->addPoint(Point(x2 - dist * cos(angle + delta), y2 - dist * sin(angle + delta)));
	s->addPoint(Point(x2, y2));

	s->addPoint(Point(x2 - dist * cos(angle - delta), y2 - dist * sin(angle - delta)));

	return s;
}

/*
 * check if something is a polygonal line with at most nsides sides
 */
int ShapeRecognizer::findPolygonal(const Point* pt, int start, int end, int nsides, int* breaks, Inertia* ss)
{
	Inertia s;
	int i1, i2, n1, n2;

	if (end == start)
	{
		return 0; // no way
	}

	if (nsides <= 0)
	{
		return 0;
	}

	if (end - start < 5)
	{
		nsides = 1; // too small for a polygon
	}

	// look for a linear piece that's big enough
	int k = 0;
	for (; k < nsides; k++)
	{
		i1 = start + (k * (end - start)) / nsides;
		i2 = start + ((k + 1) * (end - start)) / nsides;
		s.calc(pt, i1, i2);
		if (s.det() < LINE_MAX_DET)
		{
			break;
		}
	}
	if (k == nsides)
	{
		return 0; // failed!
	}

	double det1;
	double det2;
	Inertia s1;
	Inertia s2;

	// grow the linear piece we found
	while (true)
	{
		if (i1 > start)
		{
			s1 = s;
			s1.increase(pt[i1 - 1], pt[i1], 1);
			det1 = s1.det();
		}
		else
		{
			det1 = 1.0;
		}

		if (i2 < end)
		{
			s2 = s;
			s2.increase(pt[i2], pt[i2 + 1], 1);
			det2 = s2.det();
		}
		else
		{
			det2 = 1.0;
		}

		if (det1 < det2 && det1 < LINE_MAX_DET)
		{
			i1--;
			s = s1;
		}
		else if (det2 < det1 && det2 < LINE_MAX_DET)
		{
			i2++;
			s = s2;
		}
		else
		{
			break;
		}
	}

	if (i1 > start)
	{
		n1 = findPolygonal(pt, start, i1, (i2 == end) ? (nsides - 1) : (nsides - 2), breaks, ss);
		if (n1 == 0)
		{
			return 0; // it doesn't work
		}
	}
	else
	{
		n1 = 0;
	}

	breaks[n1] = i1;
	breaks[n1 + 1] = i2;
	ss[n1] = s;

	if (i2 < end)
	{
		n2 = findPolygonal(pt, i2, end, nsides - n1 - 1, breaks + n1 + 1, ss + n1 + 1);
		if (n2 == 0)
		{
			return 0;
		}
	}
	else
	{
		n2 = 0;
	}

	return n1 + n2 + 1;
}

/**
 * Improve on the polygon found by find_polygonal()
 */
void ShapeRecognizer::optimizePolygonal(const Point* pt, int nsides, int* breaks, Inertia* ss)
{
	for (int i = 1; i < nsides; i++)
	{
		// optimize break between sides i and i+1
		double cost = ss[i - 1].det() * ss[i - 1].det() + ss[i].det() * ss[i].det();
		Inertia s1 = ss[i - 1];
		Inertia s2 = ss[i];
		bool improved = false;
		while (breaks[i] > breaks[i - 1] + 1)
		{
			// try moving the break to the left
			s1.increase(pt[breaks[i] - 1], pt[breaks[i] - 2], -1);
			s2.increase(pt[breaks[i] - 1], pt[breaks[i] - 2], 1);
			double newcost = s1.det() * s1.det() + s2.det() * s2.det();

			if (newcost >= cost)
			{
				break;
			}

			improved = true;
			cost = newcost;
			breaks[i]--;
			ss[i - 1] = s1;
			ss[i] = s2;
		}

		if (improved)
		{
			continue;
		}

		s1 = ss[i - 1];
		s2 = ss[i];
		while (breaks[i] < breaks[i + 1] - 1)
		{
			// try moving the break to the right
			s1.increase(pt[breaks[i]], pt[breaks[i] + 1], 1);
			s2.increase(pt[breaks[i]], pt[breaks[i] + 1], -1);

			double newcost = s1.det() * s1.det() + s2.det() * s2.det();
			if (newcost >= cost)
			{
				break;
			}

			cost = newcost;
			breaks[i]++;
			ss[i - 1] = s1;
			ss[i] = s2;
		}
	}
}

Stroke* ShapeRecognizer::tryClosedPolygon(int nsides)
{
	//to eliminate bug #52, remove this until it's perfected
	return nullptr;

/*
	RecoSegment* r1 = nullptr;
	RecoSegment* r2 = nullptr;

	// first, we need whole strokes to combine to nsides segments...
	if (this->queueLength < nsides)
	{
		return nullptr;
	}

	RecoSegment* rs = &this->queue[this->queueLength - nsides];
	if (rs->startpt != 0)
	{
		return nullptr;
	}

	// check vertices roughly match
	for (int i = 0; i < nsides; i++)
	{
		r1 = rs + i;
		r2 = rs + (i + 1) % nsides;
		// test if r1 points away from r2 rather than towards it
		Point pt = r1->calcEdgeIsect(r2);
		r1->reversed = (hypot(pt.x - r1->x1, pt.y - r1->y1) < hypot(pt.x - r1->x2, pt.y - r1->y2));
	}

	for (int i = 0; i < nsides; i++)
	{
		r1 = rs + i;
		r2 = rs + (i + 1) % nsides;
		Point pt = r1->calcEdgeIsect(r2);
		double dist = hypot((r1->reversed ? r1->x1 : r1->x2) - pt.x,
							(r1->reversed ? r1->y1 : r1->y2) - pt.y)
								+ hypot((r2->reversed ? r2->x2 : r2->x1) - pt.x,
							(r2->reversed ? r2->y2 : r2->y1) - pt.y);
		if (dist > POLYGON_LINEAR_TOLERANCE * (r1->radius + r2->radius))
		{
			return nullptr;
		}
	}

	Stroke* s = new Stroke();
	s->applyStyleFrom(this->stroke);

	for (int i = 0; i < nsides; i++)
	{
		Point p = rs[i].calcEdgeIsect(&rs[(i + 1) % nsides]);
		s->addPoint(p);
	}

	s->addPoint(s->getPoint(0));

	return s;
*/
}

/**
 * The main pattern recognition function
 */
ShapeRecognizerResult* ShapeRecognizer::recognizePatterns(Stroke* stroke)
{
	this->stroke = stroke;

	if (stroke->getPointCount() < 3)
	{
		return nullptr;
	}

	Inertia ss[4];
	int brk[5] = {0};

	// first see if it's a polygon
	int n = findPolygonal(stroke->getPoints(), 0, stroke->getPointCount() - 1, MAX_POLYGON_SIDES, brk, ss);
	if (n > 0)
	{
		optimizePolygonal(stroke->getPoints(), n, brk, ss);
#ifdef DEBUG_RECOGNIZER
		g_message("--");
		g_message("ShapeReco:: Polygon, %d edges:", n);
		for (int i = 0; i < n; i++)
		{
			g_message("ShapeReco::      %d-%d (M=%.0f, det=%.4f)", brk[i], brk[i + 1], ss[i].getMass(), ss[i].det());
		}
		g_message("--");
#endif
		// update recognizer segment queue (most recent at end)
		while (n + queueLength > MAX_POLYGON_SIDES)
		{
			// remove oldest polygonal stroke
			int i = 1;
			while (i < queueLength && queue[i].startpt != 0)
			{
				i++;
			}
			queueLength -= i;
			g_memmove(queue, queue + i, queueLength * sizeof(RecoSegment));
		}

		RDEBUG("Queue now has %i + %i edges", this->queueLength, n);

		RecoSegment* rs = &this->queue[this->queueLength];
		this->queueLength += n;

		for (int i = 0; i < n; i++)
		{
			rs[i].startpt = brk[i];
			rs[i].endpt = brk[i + 1];
			rs[i].calcSegmentGeometry(stroke->getPoints(), brk[i], brk[i + 1], ss + i);
		}

		Stroke* tmp = nullptr;

		if ((tmp = tryRectangle()) != nullptr)
		{
			ShapeRecognizerResult* result = new ShapeRecognizerResult(tmp, this);
			resetRecognizer();
			RDEBUG("return tryRectangle()");
			return result;
		}

//		if ((tmp = tryArrow()) != nullptr)
//		{
//			ShapeRecognizerResult* result = new ShapeRecognizerResult(tmp, this);
//			resetRecognizer();
//			RDEBUG("return tryArrow()");
//			return result;
//		}
//
//		if ((tmp = tryClosedPolygon(3)) != nullptr)
//		{
//			ShapeRecognizerResult* result = new ShapeRecognizerResult(tmp, this);
//			RDEBUG("return tryClosedPolygon(3)");
//			resetRecognizer();
//			return result;
//		}
//
//		if ((tmp = tryClosedPolygon(4)) != nullptr)
//		{
//			ShapeRecognizerResult* result = new ShapeRecognizerResult(tmp, this);
//			RDEBUG("return tryClosedPolygon(4)");
//			resetRecognizer();
//			return result;
//		}


		// Removed complicated recognition

		if (n == 1) // current stroke is a line
		{
			if (fabs(rs->angle) < SLANT_TOLERANCE) // nearly horizontal
			{
				rs->angle = 0.0;
				rs->y1 = rs->y2 = rs->ycenter;
			}
			if (fabs(rs->angle) > M_PI / 2 - SLANT_TOLERANCE) // nearly vertical
			{
				rs->angle = (rs->angle > 0) ? (M_PI / 2) : (-M_PI / 2);
				rs->x1 = rs->x2 = rs->xcenter;
			}

			Stroke* s = new Stroke();
			s->applyStyleFrom(this->stroke);

			s->addPoint(Point(rs->x1, rs->y1));
			s->addPoint(Point(rs->x2, rs->y2));
			rs->stroke = s;
			ShapeRecognizerResult* result = new ShapeRecognizerResult(s);
			RDEBUG("return line");
			return result;
		}
	}

	// not a polygon: maybe a circle ?
	Stroke* s = CircleRecognizer::recognize(stroke);
	if (s)
	{
		RDEBUG("return circle");
		return new ShapeRecognizerResult(s);
	}

	return nullptr;
}
