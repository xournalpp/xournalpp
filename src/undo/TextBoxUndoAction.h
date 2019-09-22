/*
 * Xournal++
 *
 * Undo action for insert (write text, draw stroke...)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "UndoAction.h"

class Element;
class Layer;
class Redrawable;
class XojPage;

class TextBoxUndoAction : public UndoAction
{
public:
	TextBoxUndoAction(PageRef page, Layer* layer, Element* element, Element* oldelement);
	virtual ~TextBoxUndoAction();

public:
	virtual bool undo(Control* control);
	virtual bool redo(Control* control);

	virtual string getText();

private:
	Layer* layer;
	Element* element;
	Element* oldelement;
};
