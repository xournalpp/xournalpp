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

#include "control/layer/LayerController.h"

#include "UndoAction.h"

class Element;
class Layer;
class Redrawable;
class XojPage;

class LayerRenameUndoAction: public UndoAction {
public:
    LayerRenameUndoAction(Layer* layer, const string& newName, const string& oldName);
    virtual ~LayerRenameUndoAction();

public:
    std::string getText() override;
    bool undo(Control* control) override;
    bool redo(Control* control) override;

private:
    Layer* layer;
    std::string newName;
    std::string oldName;
};
