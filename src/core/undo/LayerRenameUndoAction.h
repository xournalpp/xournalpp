/*
 * Xournal++
 *
 * Rename undo action for layer
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>  // for string

#include "UndoAction.h"  // for UndoAction

class Layer;
class Control;
class LayerController;

class LayerRenameUndoAction: public UndoAction {
public:
    LayerRenameUndoAction(LayerController* layerController, Layer* layer, const std::string& newName,
                          const std::string& oldName);
    ~LayerRenameUndoAction() override;

public:
    std::string getText() override;
    bool undo(Control* control) override;
    bool redo(Control* control) override;

private:
    Layer* layer;
    LayerController* layerController;
    std::string newName;
    std::string oldName;
};
