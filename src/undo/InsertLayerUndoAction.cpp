#include "InsertLayerUndoAction.h"

#include "control/Control.h"
#include "control/layer/LayerController.h"
#include "gui/XournalView.h"
#include "model/Document.h"
#include "model/Layer.h"
#include "model/PageRef.h"

#include "i18n.h"

InsertLayerUndoAction::InsertLayerUndoAction(LayerController* layerController, const PageRef& page, Layer* layer,
                                             int layerPosition):
        UndoAction("InsertLayerUndoAction"), layerPosition(layerPosition), layerController(layerController) {
    this->page = page;
    this->layer = layer;
}

InsertLayerUndoAction::~InsertLayerUndoAction() {
    if (this->undone) {
        // The layer was undone, also deleted
        delete this->layer;
    }
}

auto InsertLayerUndoAction::getText() -> string { return _("Insert layer"); }

auto InsertLayerUndoAction::undo(Control* control) -> bool {
    // perform the same thing we did to InsertDeletePage
    // to prevent a double lock (we're already locked here)
    // doc->lock();

    layerController->removeLayer(this->page, this->layer);

    this->undone = true;

    return true;
}

auto InsertLayerUndoAction::redo(Control* control) -> bool {
    layerController->insertLayer(this->page, this->layer, layerPosition);
    Document* doc = control->getDocument();
    int id = doc->indexOf(this->page);
    control->getWindow()->getXournal()->layerChanged(id);

    this->undone = false;

    return true;
}
