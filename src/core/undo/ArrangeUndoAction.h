/*
 * Xournal++
 *
 * Undo move action (EditSelection)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <deque>    // for deque
#include <string>   // for string
#include <utility>  // for pair
#include <vector>   // for vector

#include "model/Element.h"  // for Element::Index, Element
#include "model/PageRef.h"  // for PageRef

#include "UndoAction.h"  // for UndoAction

class Control;
class Layer;

class ArrangeUndoAction: public UndoAction {
public:
    using InsertOrder = std::deque<std::pair<Element*, Element::Index>>;

    ArrangeUndoAction(const PageRef& page, Layer* layer, std::string desc, InsertOrder oldOrder, InsertOrder newOrder);
    ~ArrangeUndoAction() override;

public:
    bool undo(Control* control) override;
    bool redo(Control* control) override;
    std::string getText() override;

private:
    void applyRearrange();

private:
    std::vector<Element*> elements;
    Layer* layer;

    /** Description of the ordering action. */
    std::string description;

    // These track the ordering of elements
    InsertOrder oldOrder;
    InsertOrder newOrder;
};
