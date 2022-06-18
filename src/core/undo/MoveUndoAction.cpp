#include "MoveUndoAction.h"

#include <memory>  // for allocator, operator!=, __shared_ptr_access

#include "model/Element.h"    // for Element
#include "model/Layer.h"      // for Layer
#include "model/PageRef.h"    // for PageRef
#include "model/XojPage.h"    // for XojPage
#include "undo/UndoAction.h"  // for UndoAction
#include "util/i18n.h"        // for _

class Control;

MoveUndoAction::MoveUndoAction(Layer* sourceLayer, const PageRef& sourcePage, std::vector<Element*>* selected,
                               double mx, double my, Layer* targetLayer, PageRef targetPage):
        UndoAction("MoveUndoAction") {
    this->page = sourcePage;
    this->sourceLayer = sourceLayer;
    this->text = _("Move");

    this->dx = mx;
    this->dy = my;

    this->elements = *selected;

    if (this->page != targetPage) {
        this->targetPage = targetPage;
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
    if (this->sourceLayer != this->targetLayer && this->targetLayer != nullptr) {
        switchLayer(&this->elements, this->targetLayer, this->sourceLayer);
    }

    move();
    repaint();
    this->undone = true;

    return true;
}

auto MoveUndoAction::redo(Control* control) -> bool {
    if (this->sourceLayer != this->targetLayer && this->targetLayer != nullptr) {
        switchLayer(&this->elements, this->sourceLayer, this->targetLayer);
    }

    move();
    repaint();
    this->undone = false;

    return true;
}

void MoveUndoAction::switchLayer(std::vector<Element*>* entries, Layer* oldLayer, Layer* newLayer) {
    for (Element* e: this->elements) {
        oldLayer->removeElement(e, false);
        newLayer->addElement(e);
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
