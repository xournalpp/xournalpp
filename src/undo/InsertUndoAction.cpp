#include "InsertUndoAction.h"

#include <utility>

#include "gui/Redrawable.h"
#include "model/Element.h"
#include "model/Layer.h"
#include "model/PageRef.h"

#include "i18n.h"

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

auto InsertUndoAction::getText() -> string {
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

InsertsUndoAction::InsertsUndoAction(const PageRef& page, Layer* layer, vector<Element*> elements):
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

auto InsertsUndoAction::getText() -> string { return _("Insert elements"); }

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
