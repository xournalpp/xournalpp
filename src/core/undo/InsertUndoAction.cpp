#include "InsertUndoAction.h"

#include <memory>   // for allocator, __shared_ptr_access, __share...
#include <utility>  // for move

#include "control/Control.h"
#include "model/Document.h"
#include "model/Element.h"    // for Element, ELEMENT_IMAGE, ELEMENT_STROKE
#include "model/Layer.h"      // for Layer
#include "model/PageRef.h"    // for PageRef
#include "model/XojPage.h"    // for XojPage
#include "undo/UndoAction.h"  // for UndoAction
#include "util/i18n.h"        // for _

InsertUndoAction::InsertUndoAction(const PageRef& page, Layer* layer, const Element* element):
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
    Document* doc = control->getDocument();
    doc->lock();
    this->elementOwn = this->layer->removeElement(this->element).e;
    doc->unlock();

    this->page->fireElementChanged(this->element);

    this->undone = true;

    return true;
}

auto InsertUndoAction::redo(Control* control) -> bool {
    Document* doc = control->getDocument();
    doc->lock();
    this->layer->addElement(std::move(this->elementOwn));
    doc->unlock();

    this->page->fireElementChanged(this->element);

    this->undone = false;

    return true;
}

InsertsUndoAction::InsertsUndoAction(const PageRef& page, Layer* layer, std::vector<const Element*> elements):
        UndoAction("InsertsUndoAction"), layer(layer), elements(std::move(elements)), elementsOwn(0) {
    this->page = page;
}

InsertsUndoAction::~InsertsUndoAction() = default;

auto InsertsUndoAction::getText() -> std::string { return _("Insert elements"); }

auto InsertsUndoAction::undo(Control* control) -> bool {
    this->elementsOwn.reserve(this->elements.size());

    Document* doc = control->getDocument();
    doc->lock();
    for (const Element* elem: this->elements) {
        this->elementsOwn.emplace_back(this->layer->removeElement(elem).e);
    }
    doc->unlock();
    for (const Element* elem: this->elements) {
        this->page->fireElementChanged(elem);
    }

    this->undone = true;

    return true;
}

auto InsertsUndoAction::redo(Control* control) -> bool {
    Document* doc = control->getDocument();
    doc->lock();
    for (auto&& elem: this->elementsOwn) {
        this->layer->addElement(std::move(elem));
    }
    doc->unlock();
    this->elementsOwn = std::vector<ElementPtr>(0);

    for (auto&& elem: this->elements) {
        this->page->fireElementChanged(elem);
    }

    this->undone = false;

    return true;
}
