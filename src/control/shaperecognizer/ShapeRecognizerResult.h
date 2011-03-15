/*
 * Xournal Extended
 *
 * Xournal Shape recognizer result
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __SHAPERECOGNIZERRESULT_H__
#define __SHAPERECOGNIZERRESULT_H__

#include <glib.h>
#include "../../util/ListIterator.h"

class Stroke;
class ShapeRecognizer;

class ShapeRecognizerResult {
public:
	ShapeRecognizerResult(Stroke * result);
	ShapeRecognizerResult(Stroke * result, ShapeRecognizer * recognizer);
	~ShapeRecognizerResult();

public:
	void addSourceStroke(Stroke * s);
	Stroke * getRecognized();
	ListIterator<Stroke *> getSources();

private:
	Stroke * recognized;
	GList * source;
};

#endif /* __SHAPERECOGNIZERRESULT_H__ */
