#include "MoveLayerUndoAction.h"

#include "control/layer/LayerController.h"  // for LayerController
#include "model/Layer.h"                    // for Layer, Layer::Index
#include "model/PageRef.h"                  // for PageRef
#include "undo/UndoAction.h"                // for UndoAction
#include "util/i18n.h"                      // for _

class Control;

MoveLayerUndoAction::MoveLayerUndoAction(LayerController* layerController, const PageRef& page, Layer* layer,
                                         Layer::Index oldLayerPos, Layer::Index newLayerPos):
        UndoAction("MoveLayerUndoAction"),
        oldLayerPos(oldLayerPos),
        newLayerPos(newLayerPos),
        layerController(layerController) {
    this->page = page;
    this->layer = layer;
}

MoveLayerUndoAction::~MoveLayerUndoAction() {
    this->layerController = nullptr;
    this->layer = nullptr;
}

auto MoveLayerUndoAction::getText() -> std::string { return _("Move layer"); }

auto MoveLayerUndoAction::undo(Control* control) -> bool {
    layerController->removeLayer(this->page, this->layer);
    layerController->insertLayer(this->page, this->layer, oldLayerPos);

    this->undone = true;

    return true;
}

auto MoveLayerUndoAction::redo(Control* control) -> bool {
    layerController->removeLayer(this->page, this->layer);
    layerController->insertLayer(this->page, this->layer, newLayerPos);

    this->undone = false;

    return true;
}
