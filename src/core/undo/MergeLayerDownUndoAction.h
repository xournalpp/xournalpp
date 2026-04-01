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

#include <cstddef>  // for size_t
#include <string>   // for string

#include "model/Layer.h"    // for Layer, Layer::Index
#include "model/PageRef.h"  // for PageRef

#include "UndoAction.h"  // for UndoAction

class LayerController;
class Control;

class MergeLayerDownUndoAction: public UndoAction {
public:
    MergeLayerDownUndoAction(LayerController* layerController, const PageRef& page, Layer* upperLayer,
                             Layer::Index upperLayerPos, Layer* lowerLayer, size_t selectedPage);

public:
    bool undo(Control* control) override;
    bool redo(Control* control) override;

    std::string getText() override;

private:
    void triggerUIUpdate(Control* control);

    LayerController* layerController;

    Layer* upperLayer;
    std::vector<const Element*> upperLayerElements;
    Layer::Index upperLayerPos;
    Layer::Index upperLayerID;

    Layer* lowerLayer;
    Layer::Index lowerLayerID;

    size_t selectedPage;
};
