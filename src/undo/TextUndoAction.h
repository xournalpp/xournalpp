/*
 * Xournal++
 *
 * Undo action for text editing
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv3
 */

#ifndef __TEXTUNDOACTION_H__
#define __TEXTUNDOACTION_H__

#include "UndoAction.h"

class Layer;
class Text;
class Redrawable;
class TextEditor;

class TextUndoAction : public UndoAction
{
public:
	TextUndoAction(PageRef page, Layer* layer,
				   Text* text, string lastText,
				   TextEditor* textEditor);
	virtual ~TextUndoAction();

public:
	virtual bool undo(Control* control);
	virtual bool redo(Control* control);

	virtual string getText();

	string getUndoText();

	void textEditFinished();

private:
	XOJ_TYPE_ATTRIB;

	Layer* layer;
	Text* text;
	string lastText;
	string newText;

	TextEditor* textEditor;
};

#endif /* __TEXTUNDOACTION_H__ */
