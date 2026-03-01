#include "AddUndoAction.h"

#include <memory>  // for __shared_ptr_access, __shared_pt...

#include <glib.h>  // for g_warning

#include "control/Control.h"
#include "model/Document.h"
#include "model/Element.h"           // for Element, ELEMENT_IMAGE, ELEMENT_...
#include "model/Layer.h"             // for Layer
#include "model/XojPage.h"           // for XojPage
#include "undo/UndoAction.h"         // for UndoAction
#include "util/i18n.h"               // for _

AddUndoAction::AddUndoAction(const PageRef& page, bool eraser): UndoAction("AddUndoAction") {
    this->page = page;
    this->eraser = eraser;
}

void AddUndoAction::addElement(Layer* layer, Element* e, Element::Index pos) { elements.emplace(layer, e, pos); }

auto AddUndoAction::redo(Control* control) -> bool {
    if (elements.empty()) {
        g_warning("Could not undo AddUndoAction, there is nothing to undo");

        this->undone = true;
        return false;
    }

    Document* doc = control->getDocument();
    doc->lock();
    for (const auto& elem: elements) {
        elem.layer->insertElement(std::move(elem.elementOwn), elem.pos);
    }
    doc->unlock();
    for (const auto& elem: elements) {
        this->page->fireElementChanged(elem.element);
    }

    this->undone = true;
    return true;
}

auto AddUndoAction::undo(Control* control) -> bool {
    if (elements.empty()) {
        g_warning("Could not redo AddUndoAction, there is nothing to redo");

        this->undone = false;
        return false;
    }

    Document* doc = control->getDocument();
    doc->lock();
    for (const auto& elem: elements) {
        elem.elementOwn = elem.layer->removeElement(elem.element).e;
    }
    doc->unlock();
    for (const auto& elem: elements) {
        this->page->fireElementChanged(elem.element);
    }
    this->undone = false;

    return true;
}

auto AddUndoAction::getText() -> std::string {
    std::string text;

    if (eraser) {
        text = _("Erase stroke");
    } else {
        text = _("Paste");

        if (!elements.empty()) {
            ElementType type = elements.begin()->element->getType();

            for (auto elemIter = ++elements.begin(); elemIter != elements.end(); ++elemIter) {
                if (type != elemIter->element->getType()) {
                    text += " ";
                    text += _("elements");
                    return text;
                }
            }

            text += " ";
            switch (type) {
                case ELEMENT_STROKE:
                    text += _("stroke");
                    break;
                case ELEMENT_IMAGE:
                    text += _("image");
                    break;
                case ELEMENT_TEXIMAGE:
                    text += _("latex");
                    break;
                case ELEMENT_TEXT:
                    text += _("text");
                    break;
                case ELEMENT_LINK:
                    text += _("link");
                    break;
            }
        }
    }
    return text;
}
