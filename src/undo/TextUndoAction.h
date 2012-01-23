/*
 * Xournal++
 *
 * Undo action for text editing
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __TEXTUNDOACTION_H__
#define __TEXTUNDOACTION_H__

#include "UndoAction.h"

class Layer;
class Text;
class Redrawable;
class TextEditor;

class TextUndoAction: public UndoAction {
public:
	TextUndoAction(PageRef page, Layer * layer, Text * text, String lastText, Redrawable * view,
			TextEditor * textEditor);
	virtual ~TextUndoAction();

public:
	virtual bool undo(Control * control);
	virtual bool redo(Control * control);

	virtual String getText();

	String getUndoText();

	void textEditFinished();

private:
	XOJ_TYPE_ATTRIB;

	Layer * layer;
	Text * text;
	String lastText;
	String newText;
	Redrawable * view;

	TextEditor * textEditor;
};

#endif /* __TEXTUNDOACTION_H__ */

