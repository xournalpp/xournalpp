/*
 * Xournal++
 *
 * Part of the Xournal shape recognizer
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __RECOSEGMENT_H__
#define __RECOSEGMENT_H__

class Stroke;
class Inertia;

#include "../../model/Point.h"
#include "../../util/XournalType.h"

class RecoSegment {
public:
	RecoSegment();
	virtual ~RecoSegment();

public:
	Point calcEdgeIsect(RecoSegment * r2);

	/*
	 * find the geometry of a recognized segment
	 */
	void calcSegmentGeometry(const Point * pt, int start, int end, Inertia * s);

public:
	XOJ_TYPE_ATTRIB;

	Stroke * stroke;
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

#endif /* __RECOSEGMENT_H__ */
