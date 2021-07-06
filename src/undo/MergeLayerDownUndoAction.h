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
#include "XournalType.h"

class Layer;
class LayerController;

class MergeLayerDownUndoAction: public UndoAction {
public:
    MergeLayerDownUndoAction(LayerController* layerController, const PageRef& page, Layer* upperLayer,
                             Layer* lowerLayer, int upperLayerPos);

    // TODO: Did not copy non-default constructor definition:
    //       There is a non-default destructor defined in
    //       'src/undo/MoveLayerUndoAction.{h,cpp}' which simply sets all
    //       pointer members to `nullptr`. Is there any reason to do this?
    //       All I can think of is assuming that some other part of our program
    //       still holds a pointer to this destroyed undo/redo action and if its
    //       access doesn't segfault it might try to access these old non-null
    //       pointers which could then cause a segfault one level deeper.
    //       Seems convoluted to me, but maybe I'm missing something?

public:
    bool undo(Control* control) override;
    bool redo(Control* control) override;

    string getText() override;

private:
    const int upperLayerPos;
    LayerController* layerController;
    Layer* upperLayer;
    Layer* lowerLayer;
    const int upperLayerID;
    const int lowerLayerID;
};
