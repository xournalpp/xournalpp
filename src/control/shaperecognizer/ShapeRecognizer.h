/*
 * Xournal++
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

#include <XournalType.h>
#include <glib.h>

class Stroke;
class Point;
class ShapeRecognizerResult;

class ShapeRecognizer
{
public:
	ShapeRecognizer();
	virtual ~ShapeRecognizer();

	ShapeRecognizerResult* recognizePatterns(Stroke* stroke);
	void resetRecognizer();
private:
	Stroke* tryRectangle();
	Stroke* tryArrow();

	Stroke* tryClosedPolygon(int nsides);
	void optimizePolygonal(const Point* pt, int nsides, int* breaks, Inertia* ss);

	int findPolygonal(const Point* pt, int start, int end, int nsides, int* breaks,
					Inertia* ss);

private:
	XOJ_TYPE_ATTRIB;

	RecoSegment queue[MAX_POLYGON_SIDES + 1];
	int queueLength;

	Stroke* stroke;

	friend class ShapeRecognizerResult;
};

#endif /* __SHAPERECOGNIZER_H__ */
