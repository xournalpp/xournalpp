#include "ShapeRecognizerResult.h"

#include "ShapeRecognizer.h"
#include <Stacktrace.h>

ShapeRecognizerResult::ShapeRecognizerResult(Stroke* result)
{
	this->recognized = result;
}

ShapeRecognizerResult::ShapeRecognizerResult(Stroke* result, ShapeRecognizer* recognizer)
{
	this->recognized = result;

	for (int i = 0; i < recognizer->queueLength; i++)
	{
		if (recognizer->queue[i].stroke)
		{
			this->addSourceStroke(recognizer->queue[i].stroke);
		}
	}

	RDEBUG("source list length: %i", (int)this->source.size());
}

ShapeRecognizerResult::~ShapeRecognizerResult()
{
	this->recognized = nullptr;
}

void ShapeRecognizerResult::addSourceStroke(Stroke* s)
{
	for (Stroke* elem : this->source)
	{
		if (s == elem)
		{
			// this is a bug in the ShapreRecognizer
			// Ignore
			return;
		}
	}


	this->source.push_back(s);
}

Stroke* ShapeRecognizerResult::getRecognized()
{
	return this->recognized;
}

vector<Stroke*>* ShapeRecognizerResult::getSources()
{
	return &this->source;
}
