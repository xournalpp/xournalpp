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
// TODO: AA: type check

#ifndef __INSERTLAYERUNDOACTION_H__
#define __INSERTLAYERUNDOACTION_H__

#include "UndoAction.h"

class Layer;

class InsertLayerUndoAction: public UndoAction {
public:
	InsertLayerUndoAction(XojPage * page, Layer * layer);
	~InsertLayerUndoAction();

	virtual bool undo(Control * control);
	virtual bool redo(Control * control);

	virtual String getText();
private:
	Layer * layer;
};

#endif /* __INSERTLAYERUNDOACTION_H__ */
