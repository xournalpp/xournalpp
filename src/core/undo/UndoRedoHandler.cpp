#include "UndoRedoHandler.h"

#include <algorithm>  // for find_if
#include <cinttypes>  // for PRIu64
#include <cstdint>    // for uint64_t
#include <iterator>   // for end, begin
#include <memory>     // for unique_ptr, allocator_traits<>::value_type
#include <utility>    // for move

#include <glib.h>  // for g_message

#include "control/Control.h"  // for Control
#include "model/Document.h"   // for Document
#include "undo/UndoAction.h"  // for UndoActionPtr, UndoAction
#include "util/Assert.h"      // for xoj_assert
#include "util/XojMsgBox.h"   // for XojMsgBox
#include "util/i18n.h"        // for _, FS, _F

#include "config-debug.h"  // for DEBUG_UNDO_TRACE

static constexpr size_t UNDO_STACK_MAX_SIZE = 128;

template <typename PtrType>
inline void printAction(PtrType& action, size_t n = 666) {
    if (action) {
        g_message("%3zu -- %p / %s", n, std::to_address(action), action->getClassName().c_str());
    } else {
        g_message("%3zu -- (null)", n);
    }
}

auto UndoRedoHandler::UndoStack::next(iterator it) -> iterator {
    ++it;
    if (it == operations.end()) {
        it = operations.begin();
    }
    return it;
}

auto UndoRedoHandler::UndoStack::prev(iterator it) -> iterator {
    if (it == operations.begin()) {
        it = operations.end();
    }
    return std::prev(it);
}

UndoRedoHandler::UndoStack::UndoStack():
        operations(std::vector<UndoActionPtr>(UNDO_STACK_MAX_SIZE)),
        start(operations.begin()),
        head(operations.begin()),
        end(operations.begin()) {}


void UndoRedoHandler::printContents() {
#ifdef DEBUG_UNDO_TRACE
    size_t n = 0;
    g_message("undoList");
    for (auto it = stack.start; it != stack.head; it = stack.next(it)) {
        printAction(*it, ++n);
    }
    n = 0;
    g_message("redoList");
    for (auto it = stack.head; it != stack.end; it = stack.next(it)) {
        printAction(*it, ++n);
    }
    g_message("savedUndo");
    if (this->savedUndo) {
        printAction(this->savedUndo);
    }
#endif
}

UndoRedoHandler::UndoRedoHandler(Control* control): control(control) {}

UndoRedoHandler::~UndoRedoHandler() { clearContents(); }

void UndoRedoHandler::clearContents() {
#ifdef DEBUG_UNDO_TRACE
    for (auto it = stack.start; it != stack.end; it = stack.next(it)) {
        g_message("clearContents()::Delete UndoAction: %p / %s", it->get(), (*it)->getClassName().c_str());
    }
#endif  // DEBUG_UNDO_TRACE

    stack = UndoStack();

    this->savedUndo = nullptr;
    this->autosavedUndo = nullptr;

    printContents();
}

void UndoRedoHandler::clearRedo() {
    for (auto it = stack.head; it != stack.end; it = stack.next(it)) {
#ifdef DEBUG_UNDO_TRACE
        g_message("clearRedo()::Delete UndoAction: %p / %s", it->get(), (*it)->getClassName().c_str());
#endif
        it->reset();
    }
    stack.end = stack.head;

    printContents();
}

void UndoRedoHandler::undo() {
    if (stack.start == stack.head) {
        g_warning("UndoRedoHandler::undo() but nothing to undo");
        return;
    }

    stack.head = stack.prev(stack.head);
    xoj_assert(*stack.head);
    auto& undoAction = *(*stack.head);

    bool undoResult = undoAction.undo(this->control);

    if (!undoResult) {
        std::string msg = FS(_F("Could not undo \"{1}\"\n"
                                "Something went wrong… Please write a bug report…") %
                             undoAction.getText());
        XojMsgBox::showErrorToUser(control->getGtkWindow(), msg);
    }

    fireUpdateUndoRedoButtons(undoAction.getPages());

    printContents();
}

void UndoRedoHandler::redo() {
    if (stack.head == stack.end) {
        g_warning("UndoRedoHandler::redo() but nothing to redo");
        return;
    }

    xoj_assert(*stack.head);
    auto& redoAction = *(*stack.head);
    stack.head = stack.next(stack.head);

    bool redoResult = redoAction.redo(this->control);

    if (!redoResult) {
        std::string msg = FS(_F("Could not redo \"{1}\"\n"
                                "Something went wrong… Please write a bug report…") %
                             redoAction.getText());
        XojMsgBox::showErrorToUser(control->getGtkWindow(), msg);
    }

    fireUpdateUndoRedoButtons(redoAction.getPages());

    printContents();
}

auto UndoRedoHandler::canUndo() -> bool { return stack.head != stack.start; }

auto UndoRedoHandler::canRedo() -> bool { return stack.head != stack.end; }

/**
 * Adds an undo Action to the list, or if nullptr does nothing
 */
void UndoRedoHandler::addUndoAction(UndoActionPtr action) {
    if (!action) {
        return;
    }

    clearRedo();

    auto& a = *action;
    *stack.head = std::move(action);
    stack.head = stack.next(stack.head);
    stack.end = stack.head;

    if (stack.head == stack.start) {
        // The stack is full, we drop the oldest entry to make place for next time
        stack.start = stack.next(stack.start);
#ifdef DEBUG_UNDO_TRACE
        // the stack is full, we will overwrite this entry
        g_message("addUndoAction()::Delete UndoAction: %p / %s", stack.head->get(),
                  (*stack.head)->getClassName().c_str());
#endif  // DEBUG_UNDO_TRACE
        stack.head->reset();
    }

    fireUpdateUndoRedoButtons(a.getPages());

    printContents();
}

auto UndoRedoHandler::undoDescription() -> std::string {
    if (canUndo()) {
        UndoAction& a = **stack.prev(stack.head);
        if (!a.getText().empty()) {
            std::string txt = _("Undo: ");
            txt += a.getText();
            return txt;
        }
    }
    return _("Undo");
}

auto UndoRedoHandler::redoDescription() -> std::string {
    if (canRedo()) {
        UndoAction& a = **stack.head;
        if (!a.getText().empty()) {
            std::string txt = _("Redo: ");
            txt += a.getText();
            return txt;
        }
    }
    return _("Redo");
}

void UndoRedoHandler::fireUpdateUndoRedoButtons(const std::vector<PageRef>& pages) {
    for (auto&& undoRedoListener: this->listener) { undoRedoListener->undoRedoChanged(); }

    for (PageRef page: pages) {
        if (!page) {
            continue;
        }

        for (auto&& undoRedoListener: this->listener) { undoRedoListener->undoRedoPageChanged(page); }
    }
}

void UndoRedoHandler::addUndoRedoListener(UndoRedoListener* listener) { this->listener.emplace_back(listener); }

auto UndoRedoHandler::isChanged() -> bool {
    if (!canUndo()) {
        return this->savedUndo;
    }

    return this->savedUndo != stack.prev(stack.head)->get();
}

auto UndoRedoHandler::isChangedAutosave() -> bool {
    if (!canUndo()) {
        return this->autosavedUndo;
    }
    return this->autosavedUndo != stack.prev(stack.head)->get();
}

void UndoRedoHandler::documentAutosaved() { this->autosavedUndo = stack.prev(stack.head)->get(); }

void UndoRedoHandler::documentSaved() { this->savedUndo = stack.prev(stack.head)->get(); }
