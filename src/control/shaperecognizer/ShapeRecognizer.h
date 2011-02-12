/*
 * Xournal Extended
 *
 * Xournal Shape recognizer
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __SHAPERECOGNIZER_H__
#define __SHAPERECOGNIZER_H__

#include "RecoSegment.h"
#include "ShapeRecognizerConfig.h"
#include "CircleRecognizer.h"

class Stroke;
class Point;

class ShapeRecognizer {
public:
	ShapeRecognizer();
	virtual ~ShapeRecognizer();

	Stroke * recognizePatterns(Stroke * stroke);
private:
	void resetRecognizer();
	Point calcEdgeIsect(RecoSegment *r1, RecoSegment *r2);
	Stroke * tryRectangle();
	Stroke * tryArrow();

	Stroke * tryClosedPolygon(int nsides);
	void optimizePolygonal(const Point * pt, int nsides, int * breaks, Inertia * ss);

	int findPolygonal(const Point * pt, int start, int end, int nsides, int * breaks, Inertia * ss);

	void getSegmentGeometry(const Point * pt, int start, int end, Inertia * s, RecoSegment * r);
private:

	RecoSegment queue[MAX_POLYGON_SIDES + 1];
	int queueLength;

	Stroke * stroke;
};

#endif /* __SHAPERECOGNIZER_H__ */
