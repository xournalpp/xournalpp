#include "RemoveLayerUndoAction.h"

#include "control/Control.h"
#include "control/layer/LayerController.h"
#include "gui/XournalView.h"
#include "model/Document.h"
#include "model/Layer.h"
#include "model/PageRef.h"

#include "i18n.h"

RemoveLayerUndoAction::RemoveLayerUndoAction(LayerController* layerController, const PageRef& page, Layer* layer,
                                             int layerPos):
        UndoAction("RemoveLayerUndoAction"), layerController(layerController) {
    this->page = page;
    this->layer = layer;
    this->layerPos = layerPos;
}

RemoveLayerUndoAction::~RemoveLayerUndoAction() {
    if (!this->undone) {
        // The layer was NOT undone, also NOT restored
        delete this->layer;
    }
    this->layer = nullptr;
}

auto RemoveLayerUndoAction::getText() -> string { return _("Delete layer"); }

auto RemoveLayerUndoAction::undo(Control* control) -> bool {
    layerController->insertLayer(this->page, this->layer, this->layerPos);
    Document* doc = control->getDocument();
    int id = doc->indexOf(this->page);
    control->getWindow()->getXournal()->layerChanged(id);
    this->undone = true;

    return true;
}

auto RemoveLayerUndoAction::redo(Control* control) -> bool {
    Document* doc = control->getDocument();
    layerController->removeLayer(page, layer);
    int id = doc->indexOf(this->page);
    control->getWindow()->getXournal()->layerChanged(id);

    this->undone = false;

    return true;
}
