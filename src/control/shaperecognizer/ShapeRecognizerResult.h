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
	vector<Stroke*>* getSources();

private:
	Stroke* recognized;
	vector<Stroke*> source;
};
