#include "UndoRedoController.h"

#include <algorithm>  // for copy, max
#include <cstddef>    // for size_t
#include <iterator>   // for back_insert_iterator, back_...

#include "control/tools/EditSelection.h"  // for EditSelection
#include "gui/MainWindow.h"               // for MainWindow
#include "gui/XournalView.h"              // for XournalView
#include "model/Document.h"               // for Document
#include "model/Layer.h"                  // for Layer
#include "model/PageRef.h"                // for PageRef
#include "undo/UndoRedoHandler.h"         // for UndoRedoHandler

#include "Control.h"  // for Control

class XojPageView;

UndoRedoController::UndoRedoController(Control* control): control(control) {}

UndoRedoController::~UndoRedoController() {
    this->control = nullptr;
    this->layer = nullptr;
    elements.clear();
}

void UndoRedoController::before() {
    EditSelection* selection = control->getWindow()->getXournal()->getSelection();
    if (selection != nullptr) {
        layer = selection->getSourceLayer();
        std::copy(selection->getElements().begin(), selection->getElements().end(), std::back_inserter(elements));
    }

    control->clearSelection();
}

void UndoRedoController::after() {
    // Restore selection, if any

    if (layer == nullptr) {
        // No layer - no selection
        return;
    }

    Document* doc = control->getDocument();

    PageRef page = control->getCurrentPage();
    size_t pageNo = doc->indexOf(page);
    XojPageView* view = control->getWindow()->getXournal()->getViewFor(pageNo);

    if (!view || !page) {
        return;
    }

    std::vector<Element*> visibleElements;
    for (Element* e: elements) {
        if (layer->indexOf(e) == -1) {
            // Element is gone - so it's not selectable
            continue;
        }

        visibleElements.push_back(e);
    }
    if (!visibleElements.empty()) {
        auto* selection = new EditSelection(control->getUndoRedoHandler(), visibleElements, view, page);
        control->getWindow()->getXournal()->setSelection(selection);
    }
}

void UndoRedoController::undo(Control* control) {
    UndoRedoController handler(control);
    handler.before();

    // Move out of text mode to allow textboxundo to work
    control->clearSelectionEndText();

    control->getUndoRedoHandler()->undo();

    handler.after();
}

void UndoRedoController::redo(Control* control) {
    UndoRedoController handler(control);
    handler.before();

    control->getUndoRedoHandler()->redo();

    handler.after();
}
