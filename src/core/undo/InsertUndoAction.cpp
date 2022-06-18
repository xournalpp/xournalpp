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
        UndoAction("InsertUndoAction") {
    this->page = page;
    this->layer = layer;
    this->element = element;
}

InsertUndoAction::~InsertUndoAction() {
    if (this->undone) {
        // Insert was undone, so this is not needed anymore
        delete this->element;
    }
    this->element = nullptr;
}

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
    this->layer->removeElement(this->element, false);

    this->page->fireElementChanged(this->element);

    this->undone = true;

    return true;
}

auto InsertUndoAction::redo(Control* control) -> bool {
    this->layer->addElement(this->element);

    this->page->fireElementChanged(this->element);

    this->undone = false;

    return true;
}

InsertsUndoAction::InsertsUndoAction(const PageRef& page, Layer* layer, std::vector<Element*> elements):
        UndoAction("InsertsUndoAction") {
    this->page = page;
    this->layer = layer;
    this->elements = std::move(elements);
}

InsertsUndoAction::~InsertsUndoAction() {
    if (this->undone) {
        // Insert was undone, so this is not needed anymore
        for (Element* e: this->elements) {
            delete e;
            e = nullptr;
        }
    }
}

auto InsertsUndoAction::getText() -> std::string { return _("Insert elements"); }

auto InsertsUndoAction::undo(Control* control) -> bool {
    for (Element* elem: this->elements) {
        this->layer->removeElement(elem, false);
        this->page->fireElementChanged(elem);
    }

    this->undone = true;

    return true;
}

auto InsertsUndoAction::redo(Control* control) -> bool {
    for (Element* elem: this->elements) {
        this->layer->addElement(elem);
        this->page->fireElementChanged(elem);
    }

    this->undone = false;

    return true;
}
