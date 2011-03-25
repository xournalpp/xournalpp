/*
 * Xournal++
 *
 * Undo action for insert  layer
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __INSERTLAYERUNDOACTION_H__
#define __INSERTLAYERUNDOACTION_H__

#include "UndoAction.h"
#include "../util/XournalType.h"

class Layer;

class InsertLayerUndoAction: public UndoAction {
public:
	InsertLayerUndoAction(XojPage * page, Layer * layer);
	virtual ~InsertLayerUndoAction();

public:
	virtual bool undo(Control * control);
	virtual bool redo(Control * control);

	virtual String getText();

private:
	XOJ_TYPE_ATTRIB;

	Layer * layer;
};

#endif /* __INSERTLAYERUNDOACTION_H__ */
