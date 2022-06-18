#include "RemoveLayerUndoAction.h"

#include "control/Control.h"                // for Control
#include "control/layer/LayerController.h"  // for LayerController
#include "gui/MainWindow.h"                 // for MainWindow
#include "gui/XournalView.h"                // for XournalView
#include "model/Document.h"                 // for Document
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
    Document* doc = control->getDocument();
    auto id = doc->indexOf(this->page);
    control->getWindow()->getXournal()->layerChanged(id);
    this->undone = true;

    return true;
}

auto RemoveLayerUndoAction::redo(Control* control) -> bool {
    Document* doc = control->getDocument();
    layerController->removeLayer(page, layer);
    auto id = doc->indexOf(this->page);
    control->getWindow()->getXournal()->layerChanged(id);

    this->undone = false;

    return true;
}
