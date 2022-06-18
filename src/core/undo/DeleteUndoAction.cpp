#include "DeleteUndoAction.h"

#include <memory>  // for __shared_ptr_access, __shared_pt...

#include <glib.h>  // for g_warning

#include "model/Element.h"           // for Element, ELEMENT_IMAGE, ELEMENT_...
#include "model/Layer.h"             // for Layer
#include "model/PageRef.h"           // for PageRef
#include "model/XojPage.h"           // for XojPage
#include "undo/PageLayerPosEntry.h"  // for PageLayerPosEntry, operator<
#include "undo/UndoAction.h"         // for UndoAction
#include "util/i18n.h"               // for _

class Control;


DeleteUndoAction::DeleteUndoAction(const PageRef& page, bool eraser): UndoAction("DeleteUndoAction"), eraser(eraser) {
    this->page = page;
}

void DeleteUndoAction::addElement(Layer* layer, Element* e, Element::Index pos) { elements.emplace(layer, e, pos); }

auto DeleteUndoAction::undo(Control*) -> bool {
    if (elements.empty()) {
        g_warning("Could not undo DeleteUndoAction, there is nothing to undo");

        this->undone = true;
        return false;
    }

    for (const auto& elem: elements) {
        elem.layer->insertElement(elem.element, elem.pos);
        this->page->fireElementChanged(elem.element);
    }

    this->undone = true;
    return true;
}

auto DeleteUndoAction::redo(Control*) -> bool {
    if (elements.empty()) {
        g_warning("Could not redo DeleteUndoAction, there is nothing to redo");

        this->undone = false;
        return false;
    }

    for (const auto& elem: elements) {
        elem.layer->removeElement(elem.element, false);
        this->page->fireElementChanged(elem.element);
    }

    this->undone = false;

    return true;
}

auto DeleteUndoAction::getText() -> std::string {
    if (eraser) {
        return _("Erase stroke");
    }

    std::string text = _("Delete");

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
        }
    }

    return text;
}
