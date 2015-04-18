/*
 * Xournal++
 *
 * Undo action for rescale (EditSelection)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2
 */

#pragma once

#include "UndoAction.h"
#include <glib.h>

class ScaleUndoAction : public UndoAction
{
public:
	ScaleUndoAction(PageRef page, ElementVector* elements,
					double x0, double y0,
					double fx, double fy);
	virtual ~ScaleUndoAction();

public:
	virtual bool undo(Control* control);
	virtual bool redo(Control* control);
	virtual string getText();

private:
	void applyScale(double fx, double fy);

private:
	XOJ_TYPE_ATTRIB;

	PageRef page;
	ElementVector elements;

	double x0;
	double y0;
	double fx;
	double fy;
};
