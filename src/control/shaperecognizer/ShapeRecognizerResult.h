/*
 * Xournal++
 *
 * Xournal Shape recognizer result
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv3
 */

#ifndef __SHAPERECOGNIZERRESULT_H__
#define __SHAPERECOGNIZERRESULT_H__

#include <glib.h>
#include <ListIterator.h>
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
	ListIterator<Stroke*> getSources();

private:
	XOJ_TYPE_ATTRIB;

	Stroke* recognized;
	GList* source;
};

#endif /* __SHAPERECOGNIZERRESULT_H__ */
