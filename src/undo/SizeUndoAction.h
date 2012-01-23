/*
 * Xournal++
 *
 * Undo action resize
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __SIZEUNDOACTION_H__
#define __SIZEUNDOACTION_H__

#include "UndoAction.h"

class Layer;
class Redrawable;
class Stroke;

class SizeUndoAction: public UndoAction {
public:
	SizeUndoAction(PageRef page, Layer * layer, Redrawable * view);
	virtual ~SizeUndoAction();

public:
	virtual bool undo(Control * control);
	virtual bool redo(Control * control);
	virtual String getText();

	void addStroke(Stroke * s, double originalWidth, double newWidt, double * originalPressure, double * newPressure,
			int pressureCount);

public:
	static double * getPressure(Stroke * s);

private:
	XOJ_TYPE_ATTRIB;

	GList * data;

	Layer * layer;
	Redrawable * view;
};

#endif /* __SIZEUNDOACTION_H__ */
