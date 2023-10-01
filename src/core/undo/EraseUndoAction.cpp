#include "EraseUndoAction.h"

#include <memory>  // for __shared_ptr_access, __shar...
#include <vector>  // for vector

#include "model/Layer.h"                  // for Layer
#include "model/Stroke.h"                 // for Stroke
#include "model/XojPage.h"                // for XojPage
#include "model/eraser/ErasableStroke.h"  // for ErasableStroke
#include "undo/PageLayerPosEntry.h"       // for PageLayerPosEntry, operator<
#include "undo/UndoAction.h"              // for UndoAction
#include "util/i18n.h"                    // for _

class Control;


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
        if (entry.element->getPath().empty()) {
            // TODO (Marmare314): is this really expected behaviour?
            continue;
        } else {
            // Remove the original and add the copy
            int pos = static_cast<int>(entry.layer->removeElement(entry.element, false));

            ErasableStroke* e = entry.element->getErasable();
            std::vector<std::unique_ptr<Stroke>> strokeList = e->getStrokes();
            for (auto& stroke: strokeList) {
                // TODO (Marmare314): should use unique_ptr in layer
                Stroke* copy = stroke.release();
                entry.layer->insertElement(copy, pos);
                this->addEdited(entry.layer, copy, pos);
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
