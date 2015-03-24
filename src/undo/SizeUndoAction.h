/*
 * Xournal++
 *
 * Undo action resize
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv3
 */

#ifndef __SIZEUNDOACTION_H__
#define __SIZEUNDOACTION_H__

#include "UndoAction.h"

class Layer;
class Redrawable;
class Stroke;

class SizeUndoAction : public UndoAction
{
public:
	SizeUndoAction(PageRef page, Layer* layer);
	virtual ~SizeUndoAction();

public:
	virtual bool undo(Control* control);
	virtual bool redo(Control* control);
	virtual string getText();

	void addStroke(Stroke* s, double originalWidth, double newWidt,
				double* originalPressure, double* newPressure,
				int pressureCount);

public:
	static double* getPressure(Stroke* s);

private:
	XOJ_TYPE_ATTRIB;

	GList* data;

	Layer* layer;
};

#endif /* __SIZEUNDOACTION_H__ */
