#include "ArrangeUndoAction.h"

#include <memory>  // for allocator, __shared_ptr_access, __share...
#include <unordered_map>

#include "model/Element.h"
#include "model/Layer.h"      // for Layer
#include "model/PageRef.h"    // for PageRef
#include "model/XojPage.h"    // for XojPage
#include "undo/UndoAction.h"  // for UndoAction

class Control;

ArrangeUndoAction::ArrangeUndoAction(const PageRef& page, Layer* layer, std::string desc, InsertionOrderRef oldOrder,
                                     InsertionOrderRef newOrder):
        UndoAction("ArrangeUndoAction"),
        layer(layer),
        description(std::move(desc)),
        oldOrder(std::move(oldOrder)),
        newOrder(std::move(newOrder)) {
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

    std::unordered_map<Element*, ElementPtr> removedElements;
    removedElements.reserve(srcOrder.size());
    for (const auto& [e, _]: srcOrder) {
        removedElements.emplace(e, layer->removeElement(e).e);
    }

    for (const auto& [e, i]: tgtOrder) {
        layer->insertElement(std::move(removedElements[e]), i);
    }

    this->page->firePageChanged();
}

std::string ArrangeUndoAction::getText() { return this->description; }
