#include "InsertUndoAction.h"

#include <memory>   // for allocator, __shared_ptr_access, __share...
#include <utility>  // for move

#include "model/Element.h"    // for Element, ELEMENT_IMAGE, ELEMENT_STROKE
#include "model/Layer.h"      // for Layer
#include "model/PageRef.h"    // for PageRef
#include "model/XojPage.h"    // for XojPage
#include "undo/UndoAction.h"  // for UndoAction
#include "util/i18n.h"        // for _

class Control;

InsertUndoAction::InsertUndoAction(const PageRef& page, Layer* layer, Element* element):
        UndoAction("InsertUndoAction"), layer(layer), element(element), elementOwn(nullptr) {
    this->page = page;
}

InsertUndoAction::~InsertUndoAction() = default;

auto InsertUndoAction::getText() -> std::string {
    switch (element->getType()) {
        case ELEMENT_STROKE:
            return _("Draw stroke");
        case ELEMENT_TEXT:
            return _("Write text");
        case ELEMENT_IMAGE:
            return _("Insert image");
        case ELEMENT_TEXIMAGE:
            return _("Insert latex");
        default:
            return "";
    }
}

auto InsertUndoAction::undo(Control* control) -> bool {
    this->elementOwn = this->layer->removeElement(this->element).e;

    this->page->fireElementChanged(this->element);

    this->undone = true;

    return true;
}

auto InsertUndoAction::redo(Control* control) -> bool {
    this->layer->addElement(std::move(this->elementOwn));

    this->page->fireElementChanged(this->element);

    this->undone = false;

    return true;
}

InsertsUndoAction::InsertsUndoAction(const PageRef& page, Layer* layer, std::vector<Element*> elements):
        UndoAction("InsertsUndoAction"), layer(layer), elements(std::move(elements)), elementsOwn(0) {
    this->page = page;
}

InsertsUndoAction::~InsertsUndoAction() = default;

auto InsertsUndoAction::getText() -> std::string { return _("Insert elements"); }

auto InsertsUndoAction::undo(Control* control) -> bool {
    this->elementsOwn.reserve(this->elements.size());
    for (Element* elem: this->elements) {
        this->elementsOwn.emplace_back(this->layer->removeElement(elem).e);
        this->page->fireElementChanged(elem);
    }

    this->undone = true;

    return true;
}

auto InsertsUndoAction::redo(Control* control) -> bool {
    for (auto&& elem: this->elementsOwn) {
        auto ptr = elem.get();
        this->layer->addElement(std::move(elem));
        this->page->fireElementChanged(ptr);
    }
    this->elementsOwn = std::vector<ElementPtr>(0);

    this->undone = false;

    return true;
}
