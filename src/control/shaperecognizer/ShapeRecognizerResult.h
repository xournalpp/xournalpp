/*
 * Xournal++
 *
 * Xournal Shape recognizer result
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <XournalType.h>

class Stroke;
typedef std::vector<Stroke*> StrokeVector;

class ShapeRecognizer;

class ShapeRecognizerResult
{
public:
	ShapeRecognizerResult(Stroke* result);
	ShapeRecognizerResult(Stroke* result, ShapeRecognizer* recognizer);
	virtual ~ShapeRecognizerResult();

public:
	void addSourceStroke(Stroke* s);
	Stroke* getRecognized();
	StrokeVector* getSources();

private:
	XOJ_TYPE_ATTRIB;

	Stroke* recognized;
	StrokeVector source;
};
