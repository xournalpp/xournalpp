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

#include <string>
#include <vector>

#include "model/Layer.h"

#include "UndoAction.h"

class LayerController;

class InsertLayerUndoAction: public UndoAction {
public:
    InsertLayerUndoAction(LayerController* layerController, const PageRef& page, Layer* layer,
                          Layer::Index layerPosition);
    ~InsertLayerUndoAction() override;

public:
    bool undo(Control* control) override;
    bool redo(Control* control) override;

    std::string getText() override;

private:
    Layer::Index layerPosition;
    LayerController* layerController;
    Layer* layer;
};
