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

#include "model/Element.h"  // for Element
#include "model/PageRef.h"  // for PageRef

#include "PageLayerPosEntry.h"  // for PageLayerPosEntry
#include "UndoAction.h"         // for UndoAction

class Control;
class Layer;

class AddUndoAction: public UndoAction {
public:
    AddUndoAction(const PageRef& page, bool eraser);

public:
    bool undo(Control*) override;
    bool redo(Control*) override;

    void addElement(Layer* layer, Element* e, Element::Index pos);

    std::string getText() override;

private:
    std::multiset<PageLayerPosEntry<Element>> elements{};
    bool eraser = false;
};
