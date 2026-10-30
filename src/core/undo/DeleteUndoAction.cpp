#include "DeleteUndoAction.h"

#include <algorithm>
#include <memory>  // for __shared_ptr_access, __shared_pt...

#include <glib.h>  // for g_warning

#include "control/Control.h"
#include "model/Document.h"
#include "model/Element.h"    // for Element, ELEMENT_IMAGE, ELEMENT_...
#include "model/Layer.h"      // for Layer
#include "model/XojPage.h"    // for XojPage
#include "undo/UndoAction.h"  // for UndoAction
#include "util/i18n.h"        // for _


DeleteUndoAction::DeleteUndoAction(const PageRef& page, bool eraser): UndoAction("DeleteUndoAction"), eraser(eraser) {
    this->page = page;
}

auto DeleteUndoAction::deleteElements(const PageRef& page, Layer* layer, const InsertionOrderRef& targets)
        -> std::unique_ptr<DeleteUndoAction> {
    if (!page || !layer || targets.empty() ||
        std::find(page->getLayers().begin(), page->getLayers().end(), layer) == page->getLayers().end()) {
        return nullptr;
    }
    InsertionOrderRef ordered = targets;
    std::sort(ordered.begin(), ordered.end());
    const auto& contents = layer->getElements();
    Element::Index previous = Element::InvalidIndex;
    for (const auto& entry: ordered) {
        if (entry.pos < 0 || static_cast<size_t>(entry.pos) >= contents.size() || entry.pos == previous ||
            contents[static_cast<size_t>(entry.pos)].get() != entry.e) {
            return nullptr;
        }
        previous = entry.pos;
    }

    // Allocate history entries before moving any owners out of the layer. The batch removal reserves its
    // result before mutation and preserves original indices, unlike successive forward vector erases.
    auto action = std::make_unique<DeleteUndoAction>(page, true);
    for (const auto& entry: ordered) {
        action->elements.emplace(layer, contents[static_cast<size_t>(entry.pos)].get(), entry.pos);
    }
    auto removed = layer->removeElementsAt(ordered);
    auto destination = action->elements.begin();
    for (auto& entry: removed) {
        destination->elementOwn = std::move(entry.e);
        ++destination;
    }
    return action;
}

void DeleteUndoAction::addElement(Layer* layer, ElementPtr e, Element::Index pos) {
    elements.emplace(layer, std::move(e), pos);
}

auto DeleteUndoAction::undo(Control* control) -> bool {
    if (elements.empty()) {
        g_warning("Could not undo DeleteUndoAction, there is nothing to undo");

        this->undone = true;
        return false;
    }

    Document* doc = control->getDocument();
    doc->lock();
    for (auto& elem: elements) {
        elem.layer->insertElement(std::move(elem.elementOwn), elem.pos);
    }
    doc->unlock();
    for (const auto& elem: elements) {
        this->page->fireElementChanged(elem.element);
    }

    this->undone = true;
    return true;
}

auto DeleteUndoAction::redo(Control* control) -> bool {
    if (elements.empty()) {
        g_warning("Could not redo DeleteUndoAction, there is nothing to redo");

        this->undone = false;
        return false;
    }

    Document* doc = control->getDocument();
    doc->lock();
    for (auto& elem: elements) {
        elem.elementOwn = elem.layer->removeElement(elem.element).e;
    }
    doc->unlock();
    for (auto& elem: elements) {
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
            case ELEMENT_LINK:
                text += _("link");
                break;
        }
    }

    return text;
}
