#include "ArrangeUndoAction.h"

#include <memory>  // for allocator, __shared_ptr_access, __share...
#include <unordered_map>

#include "control/Control.h"
#include "model/Document.h"
#include "model/Element.h"
#include "model/Layer.h"      // for Layer
#include "model/XojPage.h"    // for XojPage
#include "undo/UndoAction.h"  // for UndoAction

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
    applyRearrange(control);
    return true;
}

bool ArrangeUndoAction::redo(Control* control) {
    this->undone = false;
    applyRearrange(control);
    return true;
}

void ArrangeUndoAction::applyRearrange(Control* control) {
    // Convert source order into target order
    const auto& srcOrder = this->undone ? this->newOrder : this->oldOrder;
    const auto& tgtOrder = this->undone ? this->oldOrder : this->newOrder;

    std::unordered_map<const Element*, ElementPtr> removedElements;
    removedElements.reserve(srcOrder.size());

    Document* doc = control->getDocument();
    doc->lock();
    for (const auto& [e, _]: srcOrder) {
        removedElements.emplace(e, layer->removeElement(e).e);
    }

    for (const auto& [e, i]: tgtOrder) {
        layer->insertElement(std::move(removedElements[e]), i);
    }
    doc->unlock();

    this->page->firePageChanged();
}

std::string ArrangeUndoAction::getText() { return this->description; }
