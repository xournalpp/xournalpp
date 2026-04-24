#include "RemoveLayerUndoAction.h"

#include "control/layer/LayerController.h"  // for LayerController
#include "model/Layer.h"                    // for Layer, Layer::Index
#include "model/PageRef.h"                  // for PageRef
#include "undo/UndoAction.h"                // for UndoAction
#include "util/i18n.h"                      // for _

RemoveLayerUndoAction::RemoveLayerUndoAction(LayerController* layerController, const PageRef& page, Layer* layer,
                                             Layer::Index layerPos):
        UndoAction("RemoveLayerUndoAction"), layerController(layerController), layer(layer), layerPos(layerPos) {
    this->page = page;
}

RemoveLayerUndoAction::~RemoveLayerUndoAction() {
    if (!this->undone) {
        // The layer was NOT undone, also NOT restored
        delete this->layer;
    }
    this->layer = nullptr;
}

auto RemoveLayerUndoAction::getText() -> std::string { return _("Delete layer"); }

auto RemoveLayerUndoAction::undo(Control* control) -> bool {
    layerController->insertLayer(this->page, this->layer, this->layerPos);
    this->undone = true;

    return true;
}

auto RemoveLayerUndoAction::redo(Control* control) -> bool {
    layerController->removeLayer(page, layer);
    this->undone = false;

    return true;
}
