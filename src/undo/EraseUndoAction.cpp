#include "EraseUndoAction.h"

#include "model/Layer.h"
#include "model/Stroke.h"
#include "model/eraser/EraseableStroke.h"

#include "i18n.h"

EraseUndoAction::EraseUndoAction(const PageRef& page): UndoAction("EraseUndoAction") { this->page = page; }

void EraseUndoAction::addOriginal(Layer* layer, Stroke* element, int pos) { original.emplace(layer, element, pos); }

void EraseUndoAction::addEdited(Layer* layer, Stroke* element, int pos) { edited.emplace(layer, element, pos); }

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
            int pos = static_cast<int>(entry.layer->removeElement(entry.element, false));

            EraseableStroke* e = entry.element->getEraseable();
            std::vector<std::unique_ptr<Stroke>> strokeList = e->getStroke(entry.element);
            for (auto& stroke: strokeList) {
                // TODO (Marmare314): should use unique_ptr in layer
                Stroke* copy = stroke.release();
                entry.layer->insertElement(copy, pos);
                this->addEdited(entry.layer, copy, pos);
                pos++;
            }

            delete e;
            e = nullptr;
            entry.element->setEraseable(nullptr);
        }
    }

    this->page->firePageChanged();
}

auto EraseUndoAction::getText() -> string { return _("Erase stroke"); }

auto EraseUndoAction::undo(Control* control) -> bool {
    for (auto const& entry: edited) {
        entry.layer->removeElement(entry.element, false);
        this->page->fireElementChanged(entry.element);
    }

    for (auto const& entry: original) {
        entry.layer->insertElement(entry.element, entry.pos);
        this->page->fireElementChanged(entry.element);
    }

    this->undone = true;
    return true;
}

auto EraseUndoAction::redo(Control* control) -> bool {
    for (auto const& entry: original) {
        entry.layer->removeElement(entry.element, false);
        page->fireElementChanged(entry.element);
    }

    for (auto const& entry: edited) {
        entry.layer->insertElement(entry.element, entry.pos);
        page->fireElementChanged(entry.element);
    }

    this->undone = false;
    return true;
}
