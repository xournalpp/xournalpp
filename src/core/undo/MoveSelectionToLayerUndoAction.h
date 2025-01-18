/*
 * Xournal++
 *
 * Undo action for moving selection between layers
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <set>     // for multiset
#include <string>  // for string

#include "model/Element.h"  // for Element
#include "model/PageRef.h"  // for PageRef

#include "control/layer/LayerController.h"  // for LayerController
#include "PageLayerPosEntry.h"  // for PageLayerPosEntry
#include "UndoAction.h"  // for UndoAction

class Control;
class Layer;

class MoveSelectionToLayerUndoAction: public UndoAction {
public:
    MoveSelectionToLayerUndoAction(const PageRef& page, LayerController* layerController, Layer* oldLayer, size_t oldLayerNo, size_t newLayerNo);

public:
    bool undo(Control* control) override;
    bool redo(Control* control) override;

    void addElement(Layer* layer, Element* e, Element::Index pos);

    std::string getText() override;

private:
    std::multiset<PageLayerPosEntry<Element>> elements{};
    LayerController* layerController;
    Layer* oldLayer;
    size_t oldLayerNo;
    size_t newLayerNo;
};
