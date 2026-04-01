/*
 * Xournal++
 *
 * Undo action for insert  layer
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>  // for string

#include "model/Layer.h"    // for Layer, Layer::Index
#include "model/PageRef.h"  // for PageRef

#include "UndoAction.h"  // for UndoAction

class LayerController;
class Control;

class MoveLayerUndoAction: public UndoAction {
public:
    MoveLayerUndoAction(LayerController* layerController, const PageRef& page, Layer* layer, Layer::Index oldLayerPos,
                        Layer::Index newLayerPos);
    ~MoveLayerUndoAction() override;

public:
    bool undo(Control* control) override;
    bool redo(Control* control) override;

    std::string getText() override;

private:
    Layer::Index oldLayerPos;
    Layer::Index newLayerPos;
    LayerController* layerController;
    Layer* layer;
};
