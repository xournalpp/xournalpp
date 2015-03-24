/*
 * Xournal++
 *
 * Undo action for font changes
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv3
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

class FontUndoAction : public UndoAction
{
public:
	FontUndoAction(PageRef page, Layer* layer);
	virtual ~FontUndoAction();

public:
	virtual bool undo(Control* control);
	virtual bool redo(Control* control);
	virtual string getText();

	void addStroke(Text* e, XojFont& oldFont, XojFont& newFont);

private:
	XOJ_TYPE_ATTRIB;


	GList* data;

	Layer* layer;
};

#endif /* __FONTUNDOACTION_H__ */
