/*
 * Xournal++
 *
 * Handles the erase of stroke, in special split into different parts etc.
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef ERASEHANDLER_H_
#define ERASEHANDLER_H_

#include <XournalType.h>
#include "../../model/PageRef.h"

class DeleteUndoAction;
class EraseUndoAction;
class Layer;
class Stroke;
class ToolHandler;
class Redrawable;
class Document;
class UndoRedoHandler;
class Range;

class EraseHandler {
public:
	EraseHandler(UndoRedoHandler * undo, Document * doc, PageRef page, ToolHandler * handler, Redrawable * view);
	virtual ~EraseHandler();

public:
	void erase(double x, double y);
	void finalize();

private:
	void eraseStroke(Layer * l, Stroke * s, double x, double y, Range * range);

private:
	XOJ_TYPE_ATTRIB;


	PageRef page;
	ToolHandler * handler;
	Redrawable * view;
	Document * doc;
	UndoRedoHandler * undo;

	DeleteUndoAction * eraseDeleteUndoAction;
	EraseUndoAction * eraseUndoAction;

	double halfEraserSize;
};

#endif /* ERASEHANDLER_H_ */
