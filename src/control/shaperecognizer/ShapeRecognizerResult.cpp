#include "ShapeRecognizerResult.h"

#include "ShapeRecognizer.h"
#include <Stacktrace.h>

ShapeRecognizerResult::ShapeRecognizerResult(Stroke* result)
{
	XOJ_INIT_TYPE(ShapeRecognizerResult);

	this->recognized = result;
}

ShapeRecognizerResult::ShapeRecognizerResult(Stroke* result, ShapeRecognizer* recognizer)
{
	XOJ_INIT_TYPE(ShapeRecognizerResult);

	this->recognized = result;

	for (int i = 0; i < recognizer->queueLength; i++)
	{
		if (recognizer->queue[i].stroke)
		{
			this->addSourceStroke(recognizer->queue[i].stroke);
		}
	}

	RDEBUG("source list length: {1}") % this->source.size();
}

ShapeRecognizerResult::~ShapeRecognizerResult()
{
	XOJ_CHECK_TYPE(ShapeRecognizerResult);

	this->recognized = NULL;

	XOJ_RELEASE_TYPE(ShapeRecognizerResult);
}

void ShapeRecognizerResult::addSourceStroke(Stroke* s)
{
	XOJ_CHECK_TYPE(ShapeRecognizerResult);

	for (Stroke* elem : this->source)
	{
		if (s == elem)
		{
			// TODO LOW PRIO: this is a bug in the ShapreRecognizer!!
			//		g_warning("ShapeRecognizerResult::addSourceStroke() try to add a stroke twice!");
			//		Stacktrace::printStracktrace();
			return;
		}
	}


	this->source.push_back(s);
}

Stroke* ShapeRecognizerResult::getRecognized()
{
	XOJ_CHECK_TYPE(ShapeRecognizerResult);

	return this->recognized;
}

StrokeVector* ShapeRecognizerResult::getSources()
{
	XOJ_CHECK_TYPE(ShapeRecognizerResult);

	return &this->source;
}
