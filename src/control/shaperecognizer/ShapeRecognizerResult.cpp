#include "ShapeRecognizerResult.h"
#include "ShapeRecognizer.h"

#include "../../util/Stacktrace.h"
// TODO: AA: type check

ShapeRecognizerResult::ShapeRecognizerResult(Stroke * result) {
	this->recognized = result;
	this->source = NULL;
}

ShapeRecognizerResult::ShapeRecognizerResult(Stroke * result, ShapeRecognizer * recognizer) {
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
	this->recognized = NULL;
	g_list_free(this->source);
	this->source = NULL;
}

void ShapeRecognizerResult::addSourceStroke(Stroke * s) {
	GList * elem = g_list_find(this->source, s);
	if (elem) {
		// TODO: LOW PRIO: this is a bug in the ShapreRecognizer!!
//		g_warning("ShapeRecognizerResult::addSourceStroke() try to add a stroke twice!");
//		Stacktrace::printStracktrace();
		return;
	}


	this->source = g_list_append(this->source, s);
}

Stroke * ShapeRecognizerResult::getRecognized() {
	return this->recognized;
}

ListIterator<Stroke *> ShapeRecognizerResult::getSources() {
	return ListIterator<Stroke *> (this->source);
}

