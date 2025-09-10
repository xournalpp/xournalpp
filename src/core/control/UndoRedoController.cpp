#include "UndoRedoController.h"

#include <algorithm>  // for copy, max
#include <cstddef>    // for size_t
#include <iterator>   // for back_insert_iterator, back_...

#include "control/tools/EditSelection.h"  // for EditSelection
#include "gui/MainWindow.h"               // for MainWindow
#include "gui/XournalView.h"              // for XournalView
#include "model/Document.h"               // for Document
#include "model/ElementInsertionPosition.h"
#include "model/Layer.h"                  // for Layer
#include "model/PageRef.h"                // for PageRef
#include "model/XojPage.h"                // for XojPage
#include "undo/UndoRedoHandler.h"         // for UndoRedoHandler

#include "Control.h"  // for Control

class XojPageView;

UndoRedoController::UndoRedoController(Control* control): control(control) {}

UndoRedoController::~UndoRedoController() = default;

void UndoRedoController::before() {
    EditSelection* selection = control->getWindow()->getXournal()->getSelection();
    if (selection != nullptr) {
        layer = selection->getSourceLayer();
        elements = selection->getElementsView().clone();
    }

    control->clearSelectionEndText();
}

void UndoRedoController::after() {
    // Restore selection, if any

    if (layer == nullptr) {
        // No layer - no selection
        return;
    }

    Document* doc = control->getDocument();

    PageRef page = control->getCurrentPage();
    std::unique_lock lock(*doc);
    size_t pageNo = doc->indexOf(page);
    XojPageView* view = control->getWindow()->getXournal()->getViewFor(pageNo);

    if (!view || !page) {
        // The page may have been undone
        return;
    }

    InsertionOrderRef remainingElements;
    for (const Element* e: elements) {
        // Test, if the element has been removed since
        if (auto pos = layer->indexOf(e); pos != -1) {
            remainingElements.emplace_back(e, pos);
        }
    }
    if (!remainingElements.empty()) {
        auto removedElements = layer->removeElementsAt(remainingElements);
        lock.unlock();  // Not needed anymore. For all other paths, the lock is released via ~unique_lock()
        auto [sel, bounds] =
                SelectionFactory::createFromFloatingElements(control, page, layer, view, std::move(removedElements));
        control->getWindow()->getXournal()->setSelection(sel.release());
        page->fireRangeChanged(bounds);
    }
}

void UndoRedoController::undo(Control* control) {
    UndoRedoController handler(control);
    handler.before();

    control->getUndoRedoHandler()->undo();

    handler.after();
}

void UndoRedoController::redo(Control* control) {
    UndoRedoController handler(control);
    handler.before();

    control->getUndoRedoHandler()->redo();

    handler.after();
}
