#include "MergeLayerDownUndoAction.h"

#include <memory>  // for __shared_ptr_access
#include <utility>
#include <vector>  // for vector

#include "control/layer/LayerController.h"  // for LayerController
#include "model/Layer.h"                    // for Layer, Layer::Index
#include "model/PageRef.h"                  // for PageRef
#include "model/XojPage.h"                  // for XojPage
#include "undo/UndoAction.h"                // for UndoAction
#include "util/i18n.h"                      // for _

class Element;

namespace xoj {

auto refElementContainer(const std::vector<ElementPtr>& elements) -> std::vector<Element*>;

}  // namespace xoj

MergeLayerDownUndoAction::MergeLayerDownUndoAction(LayerController* layerController, const PageRef& page,
                                                   Layer* upperLayer, Layer::Index upperLayerPos, Layer* lowerLayer,
                                                   size_t selectedPage):
        UndoAction("MergeLayerDownUndoAction"),
        layerController(layerController),
        upperLayer(upperLayer),
        upperLayerElements(this->upperLayer->getElementsView().clone()),
        upperLayerPos(upperLayerPos),
        upperLayerID(upperLayerPos + 1),
        lowerLayer(lowerLayer),
        lowerLayerID(upperLayerPos),
        selectedPage(selectedPage) {
    this->page = page;
}

auto MergeLayerDownUndoAction::getText() -> std::string { return _("Merge layer down"); }

auto MergeLayerDownUndoAction::undo(Control* control) -> bool {
    layerController->moveElementsFromLayerToFreeLayer(this->page, this->lowerLayer, this->upperLayerElements,
                                                      this->upperLayer, this->upperLayerID);
    this->page->setSelectedLayerId(this->upperLayerID);
    this->undone = true;

    return true;
}

auto MergeLayerDownUndoAction::redo(Control* control) -> bool {
    layerController->mergeLayers(this->page, this->lowerLayer, this->upperLayer);
    this->page->setSelectedLayerId(this->lowerLayerID);
    this->undone = false;

    return true;
}
