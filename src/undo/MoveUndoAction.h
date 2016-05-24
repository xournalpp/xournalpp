/*
 * Xournal++
 *
 * Undo move action (EditSelection)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "UndoAction.h"

class Layer;
class Redrawable;
class XojPage;

class MoveUndoAction : public UndoAction
{
public:
	MoveUndoAction(Layer* sourceLayer, PageRef sourcePage, ElementVector* selected, double mx, double my,
				   Layer* targetLayer, PageRef targetPage);
	virtual ~MoveUndoAction();

public:
	virtual bool undo(Control* control);
	virtual bool redo(Control* control);
	XojPage** getPages();
	virtual string getText();

private:
	void switchLayer(ElementVector* entries, Layer* oldLayer, Layer* newLayer);
	void repaint();
	void move();

private:
	XOJ_TYPE_ATTRIB;

	ElementVector elements;
	PageRef targetPage;

	Layer* sourceLayer;
	Layer* targetLayer;

	string text;

	double dx, dy;
};
