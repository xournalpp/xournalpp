/*
 * Xournal++
 *
 * Undo action for rescale (EditSelection)
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __SCALEUNDOACTION_H__
#define __SCALEUNDOACTION_H__

#include "UndoAction.h"
#include <glib.h>

class Redrawable;

class ScaleUndoAction: public UndoAction {
public:
	ScaleUndoAction(PageRef page, Redrawable * view, GList * elements, double x0, double y0, double fx, double fy);
	virtual ~ScaleUndoAction();

public:
	virtual bool undo(Control * control);
	virtual bool redo(Control * control);
	virtual String getText();

private:
	void applayScale(double fx, double fy);

private:
	XOJ_TYPE_ATTRIB;

	PageRef page;
	Redrawable * view;
	GList * elements;

	double x0;
	double y0;
	double fx;
	double fy;
};

#endif /* __SCALEUNDOACTION_H__ */
