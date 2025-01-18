#include "LayerRenameUndoAction.h"

#include "control/Control.h"
#include "control/layer/LayerController.h"  // for LayerController
#include "model/Document.h"
#include "model/Layer.h"                    // for Layer
#include "undo/UndoAction.h"                // for UndoAction
#include "util/i18n.h"                      // for _

LayerRenameUndoAction::LayerRenameUndoAction(LayerController* layerController, Layer* layer, const std::string& newName,
                                             const std::string& oldName):
        UndoAction("LayerUndoAction"),
        layer(layer),
        layerController(layerController),
        newName(newName),
        oldName(oldName) {}

LayerRenameUndoAction::~LayerRenameUndoAction() = default;

auto LayerRenameUndoAction::getText() -> std::string { return _("Rename layer"); };

auto LayerRenameUndoAction::undo(Control* control) -> bool {
    Document* doc = control->getDocument();
    doc->lock();
    layer->setName(oldName);
    doc->unlock();
    layerController->fireRebuildLayerMenu();
    return true;
}

auto LayerRenameUndoAction::redo(Control* control) -> bool {
    Document* doc = control->getDocument();
    doc->lock();
    layer->setName(newName);
    doc->unlock();
    layerController->fireRebuildLayerMenu();
    return true;
}
