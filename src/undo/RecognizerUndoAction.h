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
class Element;
class Layer;

class RecognizerUndoAction: public UndoAction {
public:
	RecognizerUndoAction(XojPage * page, Redrawable * view, Layer * layer, Element * original, Element * recognized);
	~RecognizerUndoAction();

	virtual bool undo(Control * control);
	virtual bool redo(Control * control);

	virtual String getText();
private:
	Redrawable * view;
	Layer * layer;
	Element * original;
	Element * recognized;
};

#endif /* __RECOGNIZERUNDOACTION_H__ */
