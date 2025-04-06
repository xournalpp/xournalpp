#include "EraseUndoAction.h"

#include <memory>  // for __shared_ptr_access, __shar...
#include <vector>  // for vector

#include "control/Control.h"
#include "model/Document.h"
#include "model/Layer.h"                  // for Layer
#include "model/Stroke.h"                 // for Stroke
#include "model/XojPage.h"                // for XojPage
#include "model/eraser/ErasableStroke.h"  // for ErasableStroke
#include "undo/UndoAction.h"              // for UndoAction
#include "util/i18n.h"                    // for _


EraseUndoAction::EraseUndoAction(const PageRef& page): UndoAction("EraseUndoAction") { this->page = page; }

EraseUndoAction::~EraseUndoAction() {
    auto& notInDoc = undone ? edited : original;
    for (auto& s: notInDoc) {
        delete s.element;
    }
}

void EraseUndoAction::addOriginal(Layer* layer, Stroke* element, int pos) { original.emplace(layer, element, pos); }

void EraseUndoAction::addEdited(Layer* layer, Stroke* element, Element::Index pos) {
    edited.emplace(layer, element, pos);
}

void EraseUndoAction::removeEdited(Stroke* element) {
    for (auto entryIter = edited.begin(); entryIter != edited.end(); ++entryIter) {
        if (entryIter->element == element) {
            edited.erase(entryIter);
            return;
        }
    }
}

void EraseUndoAction::finalize() {
    for (auto const& entry: original) {
        if (entry.element->getPointCount() == 0) {
            // TODO (Marmare314): is this really expected behaviour?
            continue;
        } else {
            // Remove the original and add the copy
            auto [own, pos] = entry.layer->removeElement(entry.element);
            entry.elementOwn = std::move(own);

            ErasableStroke* e = entry.element->getErasable();
            std::vector<std::unique_ptr<Stroke>> strokeList = e->getStrokes();
            for (auto& stroke: strokeList) {
                // TODO (Marmare314): should use unique_ptr in layer
                auto copy = std::move(stroke);
                this->addEdited(entry.layer, copy.get(), pos);
                entry.layer->insertElement(std::move(copy), pos);
                pos++;
            }

            delete e;
            e = nullptr;
            entry.element->setErasable(nullptr);
        }
    }

    this->page->firePageChanged();
}

auto EraseUndoAction::getText() -> std::string { return _("Erase stroke"); }

auto EraseUndoAction::undo(Control* control) -> bool {
    Document* doc = control->getDocument();
    doc->lock();
    for (auto const& entry: edited) {
        entry.elementOwn = entry.layer->removeElement(entry.element).e;
    }
    for (auto const& entry: original) {
        entry.layer->insertElement(std::move(entry.elementOwn), entry.pos);
    }
    doc->unlock();
    for (auto const& entry: edited) {
        this->page->fireElementChanged(entry.element);
    }
    for (auto const& entry: original) {
        this->page->fireElementChanged(entry.element);
    }

    this->undone = true;
    return true;
}

auto EraseUndoAction::redo(Control* control) -> bool {
    Document* doc = control->getDocument();
    doc->lock();
    for (auto const& entry: original) {
        entry.elementOwn = entry.layer->removeElement(entry.element).e;
    }
    for (auto const& entry: edited) {
        entry.layer->insertElement(std::move(entry.elementOwn), entry.pos);
    }
    doc->unlock();
    for (auto const& entry: original) {
        this->page->fireElementChanged(entry.element);
    }
    for (auto const& entry: edited) {
        this->page->fireElementChanged(entry.element);
    }

    this->undone = false;
    return true;
}
