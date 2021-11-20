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

#include <deque>

#include "model/Layer.h"

#include "UndoAction.h"

class ArrangeUndoAction: public UndoAction {
public:
    using InsertOrder = std::deque<std::pair<Element*, Layer::ElementIndex>>;

    ArrangeUndoAction(const PageRef& page, Layer* layer, std::string desc, InsertOrder oldOrder, InsertOrder newOrder);
    virtual ~ArrangeUndoAction() override;

public:
    virtual bool undo(Control* control) override;
    virtual bool redo(Control* control) override;
    virtual std::string getText() override;

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
