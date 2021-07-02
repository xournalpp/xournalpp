/*
 * Xournal++
 *
 * Undo action for merging layer down
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

class MergeLayerDownUndoAction: public UndoAction {
public:
    MergeLayerDownUndoAction(LayerController* layerController, const PageRef& page, Layer* upperLayer,
                             Layer* lowerLayer, int upperLayerPos, size_t selectedPage);

public:
    bool undo(Control* control) override;
    bool redo(Control* control) override;

    std::string getText() override;

private:
    void triggerUIUpdate(Control* control);

    const int upperLayerPos;
    LayerController* layerController;
    Layer* upperLayer;
    Layer* lowerLayer;
    const int upperLayerID;
    const int lowerLayerID;
    const size_t selectedPage;
};
