#include "ShapeRecognizerResult.h"
#include "ShapeRecognizer.h"

#include <Stacktrace.h>

ShapeRecognizerResult::ShapeRecognizerResult(Stroke * result) {
	XOJ_INIT_TYPE(ShapeRecognizerResult);

	this->recognized = result;
	this->source = NULL;
}

ShapeRecognizerResult::ShapeRecognizerResult(Stroke * result, ShapeRecognizer * recognizer) {
	XOJ_INIT_TYPE(ShapeRecognizerResult);

	this->recognized = result;
	this->source = NULL;

	for (int i = 0; i < recognizer->queueLength; i++) {
		if (recognizer->queue[i].stroke) {
			this->addSourceStroke(recognizer->queue[i].stroke);
		}
	}

	RDEBUG("source list length: %i\n", g_list_length(this->source));
}

ShapeRecognizerResult::~ShapeRecognizerResult() {
	XOJ_CHECK_TYPE(ShapeRecognizerResult);

	this->recognized = NULL;
	g_list_free(this->source);
	this->source = NULL;

	XOJ_RELEASE_TYPE(ShapeRecognizerResult);
}

void ShapeRecognizerResult::addSourceStroke(Stroke * s) {
	XOJ_CHECK_TYPE(ShapeRecognizerResult);

	GList * elem = g_list_find(this->source, s);
	if (elem) {
		// TODO LOW PRIO: this is a bug in the ShapreRecognizer!!
//		g_warning("ShapeRecognizerResult::addSourceStroke() try to add a stroke twice!");
//		Stacktrace::printStracktrace();
		return;
	}


	this->source = g_list_append(this->source, s);
}

Stroke * ShapeRecognizerResult::getRecognized() {
	XOJ_CHECK_TYPE(ShapeRecognizerResult);

	return this->recognized;
}

ListIterator<Stroke *> ShapeRecognizerResult::getSources() {
	XOJ_CHECK_TYPE(ShapeRecognizerResult);

	return ListIterator<Stroke *> (this->source);
}

