/*
 * Xournal++
 *
 * Xournal Shape recognizer
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "CircleRecognizer.h"
#include "RecoSegment.h"
#include "ShapeRecognizerConfig.h"

#include <XournalType.h>

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

	int findPolygonal(const Point* pt, int start, int end, int nsides, int* breaks, Inertia* ss);

private:
	RecoSegment queue[MAX_POLYGON_SIDES + 1];
	int queueLength;

	Stroke* stroke;

	friend class ShapeRecognizerResult;
};
