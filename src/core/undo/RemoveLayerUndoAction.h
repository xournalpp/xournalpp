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

#include "UndoAction.h"

class Layer;
class LayerController;

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
