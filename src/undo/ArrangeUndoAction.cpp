#include "ArrangeUndoAction.h"

#include "model/Element.h"
#include "model/PageRef.h"

#include "Range.h"

ArrangeUndoAction::ArrangeUndoAction(const PageRef& page, Layer* layer, std::string desc, InsertOrder oldOrder,
                                     InsertOrder newOrder):
        UndoAction("ArrangeUndoAction"), layer(layer), description(desc), oldOrder(oldOrder), newOrder(newOrder) {
    this->page = page;
}

ArrangeUndoAction::~ArrangeUndoAction() { this->page = nullptr; }

bool ArrangeUndoAction::undo(Control* control) {
    this->undone = true;
    applyRearrange();
    return true;
}

bool ArrangeUndoAction::redo(Control* control) {
    this->undone = false;
    applyRearrange();
    return true;
}

void ArrangeUndoAction::applyRearrange() {
    // Convert source order into target order
    const auto& srcOrder = this->undone ? this->newOrder : this->oldOrder;
    const auto& tgtOrder = this->undone ? this->oldOrder : this->newOrder;

    for (const auto& [e, _]: srcOrder) {
        layer->removeElement(e, false);
    }

    for (const auto& [e, i]: tgtOrder) {
        layer->insertElement(e, i);
    }

    this->page->firePageChanged();
}

std::string ArrangeUndoAction::getText() { return this->description; }
