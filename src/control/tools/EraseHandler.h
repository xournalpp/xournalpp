/*
 * Xournal Extended
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

class DeleteUndoAction;
class EraseUndoAction;
class Layer;
class Stroke;
class XojPage;
class ToolHandler;
class PageView;
class Document;
class UndoRedoHandler;

class EraseHandler {
public:
	EraseHandler(UndoRedoHandler * undo, Document * doc, XojPage * page, ToolHandler * handler, PageView * view);
	virtual ~EraseHandler();

public:
	void erase(double x, double y);
	void finalize();

private:
	void eraseStroke(Layer * l, Stroke * s, double x, double y);

private:
	XojPage * page;
	ToolHandler * handler;
	PageView * view;
	Document * doc;
	UndoRedoHandler * undo;

	DeleteUndoAction * eraseDeleteUndoAction;
	EraseUndoAction * eraseUndoAction;

	double halfEraserSize;
};

#endif /* ERASEHANDLER_H_ */
