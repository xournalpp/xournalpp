#include "MoveUndoAction.h"

#include <memory>  // for allocator, operator!=, __shared_ptr_access
#include <utility>

#include "control/Control.h"
#include "model/Document.h"
#include "model/Element.h"    // for Element
#include "model/Layer.h"      // for Layer
#include "model/PageRef.h"    // for PageRef
#include "model/XojPage.h"    // for XojPage
#include "undo/UndoAction.h"  // for UndoAction
#include "util/i18n.h"        // for _

MoveUndoAction::MoveUndoAction(Layer* sourceLayer, const PageRef& sourcePage, std::vector<Element*> selected, double mx,
                               double my, Layer* targetLayer, PageRef targetPage):
        UndoAction("MoveUndoAction"),
        elements(std::move(selected)),
        sourceLayer(sourceLayer),
        text(_("Move")),
        dx(mx),
        dy(my) {
    this->page = sourcePage;

    if (this->page != targetPage) {
        this->targetPage = std::move(targetPage);
        this->targetLayer = targetLayer;
    }
}

MoveUndoAction::~MoveUndoAction() = default;

void MoveUndoAction::move() {
    if (this->undone) {
        for (Element* e: this->elements) { e->move(dx, dy); }
    } else {
        for (Element* e: this->elements) { e->move(-dx, -dy); }
    }
}

auto MoveUndoAction::undo(Control* control) -> bool {
    Document* doc = control->getDocument();
    doc->lock();
    if (this->sourceLayer != this->targetLayer && this->targetLayer != nullptr) {
        switchLayer(&this->elements, this->targetLayer, this->sourceLayer);
    }

    move();
    doc->unlock();
    repaint();
    this->undone = true;

    return true;
}

auto MoveUndoAction::redo(Control* control) -> bool {
    Document* doc = control->getDocument();
    doc->lock();
    if (this->sourceLayer != this->targetLayer && this->targetLayer != nullptr) {
        switchLayer(&this->elements, this->sourceLayer, this->targetLayer);
    }

    move();
    doc->unlock();
    repaint();
    this->undone = false;

    return true;
}

void MoveUndoAction::switchLayer(std::vector<Element*>* entries, Layer* oldLayer, Layer* newLayer) {
    for (Element* e: this->elements) {
        newLayer->addElement(oldLayer->removeElement(e).e);
    }
}

void MoveUndoAction::repaint() {
    if (this->elements.empty()) {
        return;
    }

    this->page->firePageChanged();

    if (this->targetPage) {
        this->targetPage->firePageChanged();
    }
}

auto MoveUndoAction::getPages() -> std::vector<PageRef> {
    std::vector<PageRef> pages;
    pages.push_back(this->page);
    pages.push_back(this->targetPage);
    return pages;
}

auto MoveUndoAction::getText() -> std::string { return text; }
