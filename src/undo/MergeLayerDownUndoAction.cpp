#include "MergeLayerDownUndoAction.h"

#include "control/Control.h"
#include "control/layer/LayerController.h"
#include "gui/XournalView.h"
#include "model/Document.h"
#include "model/Layer.h"
#include "model/PageRef.h"

#include "i18n.h"

MergeLayerDownUndoAction::MergeLayerDownUndoAction(LayerController* layerController, const PageRef& page,
                                                   Layer* upperLayer, Layer* lowerLayer, int upperLayerPos):
        UndoAction("MergeLayerDownUndoAction"), upperLayerPos(upperLayerPos), layerController(layerController) {
    this->page = page;
    this->upperLayer = upperLayer;
    this->lowerLayer = lowerLayer;
}

auto MergeLayerDownUndoAction::getText() -> string {
    return _("Merge layer down"); /* TODO: probably need to add this to some i18n file */
}

auto MergeLayerDownUndoAction::undo(Control* control) -> bool {
    // remove all elements present in the upper layer from the lower layer again
    const bool free_elems = false;  // don't free the elems, they're still used
    for (Element* elem: *(this->upperLayer->getElements())) { this->lowerLayer->removeElement(elem, free_elems); }

    // add the upper layer back at its old pos
    layerController->insertLayer(this->page, this->upperLayer, upperLayerPos);

    this->undone = true;

    return true;
}

auto MergeLayerDownUndoAction::redo(Control* control) -> bool {
    // remove the upper layer
    layerController->removeLayer(this->page, this->upperLayer);

    // add all elements back to the lower layer
    for (Element* elem: *(this->upperLayer->getElements())) { this->lowerLayer->addElement(elem); }

    this->undone = false;

    return true;
}
