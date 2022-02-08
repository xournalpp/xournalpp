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

#include "UndoAction.h"


class Layer;
class LayerController;

class InsertLayerUndoAction: public UndoAction {
public:
    InsertLayerUndoAction(LayerController* layerController, const PageRef& page, Layer* layer, int layerPosition);
    ~InsertLayerUndoAction() override;

public:
    bool undo(Control* control) override;
    bool redo(Control* control) override;

    std::string getText() override;

private:
    int layerPosition;
    LayerController* layerController;
    Layer* layer;
};
