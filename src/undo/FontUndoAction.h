/*
 * Xournal++
 *
 * Undo action for font changes
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __FONTUNDOACTION_H__
#define __FONTUNDOACTION_H__

#include "UndoAction.h"
#include <glib.h>
#include <XournalType.h>

class Layer;
class Redrawable;
class Text;
class XojFont;

class FontUndoAction: public UndoAction
{
public:
	FontUndoAction(PageRef page, Layer* layer, Redrawable* view);
	virtual ~FontUndoAction();

public:
	virtual bool undo(Control* control);
	virtual bool redo(Control* control);
	virtual String getText();

	void addStroke(Text* e, XojFont& oldFont, XojFont& newFont);

private:
	XOJ_TYPE_ATTRIB;


	GList* data;

	Layer* layer;
	Redrawable* view;
};

#endif /* __FONTUNDOACTION_H__ */
