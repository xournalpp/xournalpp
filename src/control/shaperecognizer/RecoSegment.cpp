#include "RecoSegment.h"

#include "Inertia.h"

#include <cmath>
#include <stdlib.h>

RecoSegment::RecoSegment()
{
	this->stroke = nullptr;
	this->angle = 0;
	this->endpt = 0;

	this->radius = 0;
	this->reversed = false;
	this->startpt = 0;
	this->x1 = 0;
	this->x2 = 0;
	this->xcenter = 0;
	this->y1 = 0;
	this->y2 = 0;
	this->ycenter = 0;
}

RecoSegment::~RecoSegment()
{
}

Point RecoSegment::calcEdgeIsect(RecoSegment* r2)
{
	double t;
	t = (r2->xcenter - this->xcenter) * sin(r2->angle) - (r2->ycenter - this->ycenter) * cos(r2->angle);
	t /= sin(r2->angle - this->angle);
	double x = this->xcenter + t * cos(this->angle);
	double y = this->ycenter + t * sin(this->angle);

	return Point(x, y);
}

/**
 * Find the geometry of a recognized segment
 */
void RecoSegment::calcSegmentGeometry(const Point* pt, int start, int end, Inertia* s)
{
	this->xcenter = s->centerX();
	this->ycenter = s->centerY();
	double a = s->xx();
	double b = s->xy();
	double c = s->yy();

	// max angle for inertia quadratic form solves: tan(2t) = 2b/(a-c)
	this->angle = atan2(2 * b, a - c) / 2;
	this->radius = sqrt(3 * (a + c));

	double lmin = 0;
	double lmax = 0;

	for (int i = start; i <= end; i++)
	{
		double l = (pt[i].x - this->xcenter) * cos(this->angle) + (pt[i].y - this->ycenter) * sin(this->angle);
		if (l < lmin)
		{
			lmin = l;
		}

		if (l > lmax)
		{
			lmax = l;
		}
	}

	this->x1 = this->xcenter + lmin * cos(this->angle);
	this->y1 = this->ycenter + lmin * sin(this->angle);
	this->x2 = this->xcenter + lmax * cos(this->angle);
	this->y2 = this->ycenter + lmax * sin(this->angle);
}
