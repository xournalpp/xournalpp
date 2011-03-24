/*
 * Xournal++
 *
 * Undo action for stroke recognizer
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __RECOGNIZERUNDOACTION_H__
#define __RECOGNIZERUNDOACTION_H__

#include "UndoAction.h"

class XojPage;
class Redrawable;
class Stroke;
class Layer;

class RecognizerUndoAction: public UndoAction {
public:
	RecognizerUndoAction(XojPage * page, Redrawable * view, Layer * layer, Stroke * original, Stroke * recognized);
	~RecognizerUndoAction();

public:
	void addSourceElement(Stroke * s);

	virtual bool undo(Control * control);
	virtual bool redo(Control * control);

	virtual String getText();

private:
	XOJ_TYPE_ATTRIB;

	Redrawable * view;
	Layer * layer;
	Stroke * recognized;
	GList * original;
};

#endif /* __RECOGNIZERUNDOACTION_H__ */
