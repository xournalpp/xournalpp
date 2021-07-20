/*
 * Xournal++
 *
 * Undo action for eraser
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <set>
#include <string>

#include "PageLayerPosEntry.h"
#include "UndoAction.h"

class Stroke;

class EraseUndoAction: public UndoAction {
public:
    EraseUndoAction(const PageRef& page);

public:
    bool undo(Control* control) override;
    bool redo(Control* control) override;

    void addOriginal(Layer* layer, Stroke* element, int pos);
    void addEdited(Layer* layer, Stroke* element, int pos);
    [[maybe_unused]] void removeEdited(Stroke* element);

    void finalize();

    string getText() override;

private:
    std::multiset<PageLayerPosEntry<Stroke>> edited{};
    std::multiset<PageLayerPosEntry<Stroke>> original{};
};
