#include "InsertLayerUndoAction.h"

#include "control/Control.h"                // for Control
#include "control/layer/LayerController.h"  // for LayerController
#include "gui/MainWindow.h"                 // for MainWindow
#include "gui/XournalView.h"                // for XournalView
#include "model/Document.h"                 // for Document
#include "model/Layer.h"                    // for Layer, Layer::Index
#include "model/PageRef.h"                  // for PageRef
#include "undo/UndoAction.h"                // for UndoAction
#include "util/i18n.h"                      // for _

InsertLayerUndoAction::InsertLayerUndoAction(LayerController* layerController, const PageRef& page, Layer* layer,
                                             Layer::Index layerPosition):
        UndoAction("InsertLayerUndoAction"),
        layerPosition(layerPosition),
        layerController(layerController),
        layer(layer) {
    this->page = page;
}

InsertLayerUndoAction::~InsertLayerUndoAction() {
    if (this->undone) {
        // The layer was undone, also deleted
        delete this->layer;
    }
}

auto InsertLayerUndoAction::getText() -> std::string { return _("Insert layer"); }

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
    auto id = doc->indexOf(this->page);
    control->getWindow()->getXournal()->layerChanged(id);

    this->undone = false;

    return true;
}
