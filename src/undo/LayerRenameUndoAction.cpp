#include "LayerRenameUndoAction.h"

#include "i18n.h"

LayerRenameUndoAction::LayerRenameUndoAction(Layer* layer, const std::string& newName, const std::string& oldName):
        UndoAction("LayerUndoAction"), layer(layer), newName(newName), oldName(oldName) {}

LayerRenameUndoAction::~LayerRenameUndoAction() = default;

auto LayerRenameUndoAction::getText() -> std::string { return _("Rename layer"); };

auto LayerRenameUndoAction::undo(Control* control) -> bool {
    layer->setName(oldName);
    control->getLayerController()->setCurrentLayerName(layer->getName());
    return true;
}

auto LayerRenameUndoAction::redo(Control* control) -> bool {
    layer->setName(newName);
    control->getLayerController()->setCurrentLayerName(layer->getName());
    return true;
}
