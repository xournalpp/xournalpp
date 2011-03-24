/*
 * Xournal++
 *
 * Undo move action (EditSelection)
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __MOVEUNDOACTION_H__
#define __MOVEUNDOACTION_H__

#include "UndoAction.h"
#include <glib.h>

class EditSelection;
class VerticalToolHandler;
class XojPage;
class Layer;
class Redrawable;

class MoveUndoAction: public UndoAction {
public:
	MoveUndoAction(XojPage * page, EditSelection * selection);
	MoveUndoAction(XojPage * page, VerticalToolHandler * handler);
	~MoveUndoAction();

	void finalize(EditSelection * selection);
	void finalize(VerticalToolHandler * handler);

	virtual bool undo(Control * control);
	virtual bool redo(Control * control);
	XojPage ** getPages();
	virtual String getText();

private:
	void acceptPositions(GList * pos);
	void switchLayer(GList * entries, Layer * oldLayer, Layer * newLayer);
	void repaint(Redrawable * view, GList * list);
	void repaint();

private:
	XOJ_TYPE_ATTRIB;

	GList * originalPos;
	GList * newPos;
	XojPage * newPage;

	Layer * oldLayer;
	Layer * newLayer;

	Redrawable * origView;
	Redrawable * newView;

	String text;

	friend class EditSelection;
};

#endif /* __MOVEUNDOACTION_H__ */
