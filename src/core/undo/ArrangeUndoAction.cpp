#include "ArrangeUndoAction.h"

#include <memory>  // for allocator, __shared_ptr_access, __share...

#include "model/Layer.h"      // for Layer
#include "model/PageRef.h"    // for PageRef
#include "model/XojPage.h"    // for XojPage
#include "undo/UndoAction.h"  // for UndoAction

class Control;

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

    for (const auto& [e, _]: srcOrder) { layer->removeElement(e, false); }

    for (const auto& [e, i]: tgtOrder) { layer->insertElement(e, i); }

    this->page->firePageChanged();
}

std::string ArrangeUndoAction::getText() { return this->description; }
