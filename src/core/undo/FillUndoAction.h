/*
 * Xournal++
 *
 * Undo action resize
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "UndoAction.h"

class Layer;
class FillUndoActionEntry;
class Stroke;

class FillUndoAction: public UndoAction {
public:
    FillUndoAction(const PageRef& page, Layer* layer);
    ~FillUndoAction() override;

public:
    bool undo(Control* control) override;
    bool redo(Control* control) override;
    std::string getText() override;

    void addStroke(Stroke* s, int originalFill, int newFill);

private:
    std::vector<FillUndoActionEntry*> data;

    Layer* layer;
};
