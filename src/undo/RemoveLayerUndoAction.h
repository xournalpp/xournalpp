/*
 * Xournal++
 *
 * Undo action for remove layer
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __REMOVELAYERUNDOACTION_H__
#define __REMOVELAYERUNDOACTION_H__

#include "UndoAction.h"

class Layer;

class RemoveLayerUndoAction: public UndoAction {
public:
	RemoveLayerUndoAction(PageRef page, Layer * layer, int layerPos);
	virtual ~RemoveLayerUndoAction();

public:
	virtual bool undo(Control * control);
	virtual bool redo(Control * control);

	virtual String getText();

private:
	XOJ_TYPE_ATTRIB;

	Layer * layer;
	int layerPos;
};

#endif /* __REMOVELAYERUNDOACTION_H__ */
