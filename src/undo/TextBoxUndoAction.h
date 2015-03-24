/*
 * Xournal++
 *
 * Undo action for insert (write text, draw stroke...)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv3
 */

#ifndef __TEXTBOXUNDOACTION_H__
#define __TEXTBOXUNDOACTION_H__

#include "UndoAction.h"

class XojPage;
class Layer;
class Element;
class Redrawable;

class TextBoxUndoAction : public UndoAction
{
public:
	TextBoxUndoAction(PageRef page, Layer* layer, Element* element,
					  Element* oldelement);
	virtual ~TextBoxUndoAction();

public:
	virtual bool undo(Control* control);
	virtual bool redo(Control* control);

	virtual string getText();

private:
	XOJ_TYPE_ATTRIB;

	Layer* layer;
	Element* element;
	Element* oldelement;
};

#endif /* __TEXTBOXUNDOACTION_H__ */
