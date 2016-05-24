#include "Inertia.h"

#include "model/Point.h"

#include <glib.h>
#include <math.h>

Inertia::Inertia()
{
	XOJ_INIT_TYPE(Inertia);

	this->mass = 0;
	this->sx = 0;
	this->sxx = 0;
	this->sxy = 0;
	this->sy = 0;
	this->syy = 0;
}

Inertia::Inertia(const Inertia& inertia)
{
	XOJ_CHECK_TYPE_OBJ(&inertia, Inertia);

	XOJ_INIT_TYPE(Inertia);
	*this = inertia;
}

Inertia::~Inertia()
{
	XOJ_RELEASE_TYPE(Inertia);
}

double Inertia::centerX()
{
	XOJ_CHECK_TYPE(Inertia);

	return this->sx / this->mass;
}

double Inertia::centerY()
{
	XOJ_CHECK_TYPE(Inertia);

	return this->sy / this->mass;
}

double Inertia::xx()
{
	XOJ_CHECK_TYPE(Inertia);

	if (this->mass <= 0.0)
	{
		return 0.0;
	}
	return (this->sxx - this->sx * this->sx / this->mass) / this->mass;
}

double Inertia::xy()
{
	XOJ_CHECK_TYPE(Inertia);

	if (this->mass <= 0.0)
	{
		return 0.0;
	}
	return (this->sxy - this->sx * this->sy / this->mass) / this->mass;
}

double Inertia::yy()
{
	XOJ_CHECK_TYPE(Inertia);

	if (this->mass <= 0.0)
	{
		return 0.0;
	}
	return (this->syy - this->sy * this->sy / this->mass) / this->mass;
}

double Inertia::rad()
{
	XOJ_CHECK_TYPE(Inertia);

	double ixx = this->xx();
	double iyy = this->yy();
	if (ixx + iyy <= 0.0)
	{
		return 0.0;
	}
	return sqrt(ixx + iyy);
}

double Inertia::det()
{
	XOJ_CHECK_TYPE(Inertia);

	double ixx = this->xx();
	double iyy = this->yy();
	double ixy = this->xy();
	if (this->mass <= 0.0)
	{
		return 0.0;
	}

	if (ixx + iyy <= 0.0)
	{
		return 0.0;
	}

	return 4 * (ixx * iyy - ixy * ixy) / (ixx + iyy) / (ixx + iyy);
}

double Inertia::getMass()
{
	XOJ_CHECK_TYPE(Inertia);

	return mass;
}

void Inertia::increase(Point p1, Point p2, int coef)
{
	XOJ_CHECK_TYPE(Inertia);

	double dm = coef * hypot(p2.x - p1.x, p2.y - p1.y);
	this->mass += dm;
	this->sx += dm * p1.x;
	this->sy += dm * p1.y;
	this->sxx += dm * p1.x * p1.x;
	this->syy += dm * p1.y * p1.y;
	this->sxy += dm * p1.x * p1.y;
}

void Inertia::calc(const Point* pt, int start, int end)
{
	XOJ_CHECK_TYPE(Inertia);

	this->mass = this->sx = this->sy = this->sxx = this->sxy = this->syy = 0.;
	for (int i = start; i < end - 1; i++)
	{
		this->increase(pt[i], pt[i + 1], 1);
	}
}
