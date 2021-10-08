#include "LayerRenameUndoAction.h"

#include "i18n.h"

LayerRenameUndoAction::LayerRenameUndoAction(LayerController* layerController, Layer* layer, const std::string& newName,
                                             const std::string& oldName):
        UndoAction("LayerUndoAction"),
        layerController(layerController),
        layer(layer),
        newName(newName),
        oldName(oldName) {}

LayerRenameUndoAction::~LayerRenameUndoAction() = default;

auto LayerRenameUndoAction::getText() -> std::string { return _("Rename layer"); };

auto LayerRenameUndoAction::undo(Control* control) -> bool {
    layer->setName(oldName);
    layerController->fireRebuildLayerMenu();
    return true;
}

auto LayerRenameUndoAction::redo(Control* control) -> bool {
    layer->setName(newName);
    layerController->fireRebuildLayerMenu();
    return true;
}
