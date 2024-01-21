/*
 * Xournal++
 *
 * Undo action for delete (eraser, delete)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <set>     // for multiset
#include <string>  // for string

#include "model/Element.h"  // for Element, Element::Index
#include "model/PageRef.h"  // for PageRef

#include "PageLayerPosEntry.h"  // for PageLayerPosEntry
#include "UndoAction.h"         // for UndoAction

class Control;
class Layer;

class DeleteUndoAction: public UndoAction {
public:
    DeleteUndoAction(const PageRef& page, bool eraser);

public:
    bool undo(Control* control) override;
    bool redo(Control* control) override;

    void addElement(Layer* layer, ElementPtr e, Element::Index pos);

    std::string getText() override;

private:
    // Todo (performance): replace by flat_multi_set / sorted_vector
    std::multiset<PageLayerPosEntry<Element>> elements{};
    bool eraser = true;
};
