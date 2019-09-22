/*
 * Xournal++
 *
 * Part of the Xournal shape recognizer
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "model/Point.h"

#include <XournalType.h>

class Stroke;
class Inertia;

class RecoSegment
{
public:
	RecoSegment();
	virtual ~RecoSegment();

public:
	Point calcEdgeIsect(RecoSegment* r2);

	/**
	 * Find the geometry of a recognized segment
	 */
	void calcSegmentGeometry(const Point* pt, int start, int end, Inertia* s);

public:
	Stroke* stroke;
	int startpt;
	int endpt;

	double xcenter;
	double ycenter;
	double angle;
	double radius;

	double x1;
	double y1;
	double x2;
	double y2;

	bool reversed;
};
