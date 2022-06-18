/*
 * Xournal++
 *
 * Undo action for remove layer
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

class RemoveLayerUndoAction: public UndoAction {
public:
    RemoveLayerUndoAction(LayerController* layerController, const PageRef& page, Layer* layer, Layer::Index layerPos);
    ~RemoveLayerUndoAction() override;

public:
    bool undo(Control* control) override;
    bool redo(Control* control) override;

    std::string getText() override;

private:
    LayerController* layerController;
    Layer* layer;
    Layer::Index layerPos;
};
