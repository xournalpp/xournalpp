#include "UndoRedoHandler.h"

#include <algorithm>  // for find_if
#include <cinttypes>  // for PRIu64
#include <cstdint>    // for uint64_t
#include <iterator>   // for end, begin
#include <memory>     // for unique_ptr, allocator_traits<>::value_type
#include <utility>    // for move

#include <glib.h>  // for g_message, g_assert_true

#include "control/Control.h"  // for Control
#include "model/Document.h"   // for Document
#include "undo/UndoAction.h"  // for UndoActionPtr, UndoAction
#include "util/XojMsgBox.h"   // for XojMsgBox
#include "util/i18n.h"        // for _, FS, _F

using std::string;


template <typename T>
T* GetPtr(T* ptr) {
    return ptr;
}

template <typename T>
T* GetPtr(const std::unique_ptr<T>& ptr) {
    return ptr.get();
}

template <typename PtrType>
inline void printAction(PtrType& action, size_t n = 666) {
    if (action) {
        g_message("%3zu -- %p / %s", n, GetPtr(action), action->getClassName().c_str());
    } else {
        g_message("%3zu -- (null)", n);
    }
}

template <typename PtrType>
inline void printUndoList(const std::deque<PtrType>& list) {
    size_t n = 1;
    for (const auto& action: list) {
        printAction(action, n++);
    }
}

#ifdef UNDO_TRACE
constexpr bool UNDO_TRACE_BOOL = true;
#else
constexpr bool UNDO_TRACE_BOOL = false;
#endif

void UndoRedoHandler::printContents() {
    if constexpr (UNDO_TRACE_BOOL)  // NOLINT
    {
        g_message("redoList");             // NOLINT
        printUndoList(this->redoList);     // NOLINT
        g_message("undoList");             // NOLINT
        printUndoList(this->undoList);     // NOLINT
        g_message("savedUndo");            // NOLINT
        if (this->savedUndo)               // NOLINT
        {                                  // NOLINT
            printAction(this->savedUndo);  // NOLINT
        }                                  // NOLINT
    }
}

UndoRedoHandler::UndoRedoHandler(Control* control): control(control) {}

UndoRedoHandler::~UndoRedoHandler() { clearContents(); }

void UndoRedoHandler::clearContents() {
#ifdef UNDO_TRACE
    for (auto const& undoAction: this->undoList) {
        g_message("clearContents()::Delete UndoAction: %p / %s", undoAction.get(), undoAction->getClassName().c_str());
    }
#endif  // UNDO_TRACE

    undoList.clear();
    clearRedo();

    this->savedUndo = nullptr;
    this->autosavedUndo = nullptr;

    printContents();
}

void UndoRedoHandler::clearRedo() {
#ifdef UNDO_TRACE
    for (auto const& undoAction: this->redoList) {
        g_message("clearRedo()::Delete UndoAction: %p / %s", undoAction.get(), undoAction->getClassName().c_str());
    }
#endif
    redoList.clear();
    printContents();
}

void UndoRedoHandler::undo() {
    if (this->undoList.empty()) {
        return;
    }

    g_assert_true(this->undoList.back());

    auto& undoAction = *this->undoList.back();
    this->redoList.emplace_back(std::move(this->undoList.back()));
    this->undoList.pop_back();

    Document* doc = control->getDocument();
    doc->lock();
    bool undoResult = undoAction.undo(this->control);
    doc->unlock();

    if (!undoResult) {
        string msg = FS(_F("Could not undo \"{1}\"\n"
                           "Something went wrong… Please write a bug report…") %
                        undoAction.getText());
        XojMsgBox::showErrorToUser(control->getGtkWindow(), msg);
    }

    fireUpdateUndoRedoButtons(undoAction.getPages());

    printContents();
}

void UndoRedoHandler::redo() {
    if (this->redoList.empty()) {
        return;
    }

    g_assert_true(this->redoList.back());

    UndoAction& redoAction = *this->redoList.back();

    this->undoList.emplace_back(std::move(this->redoList.back()));
    this->redoList.pop_back();

    Document* doc = control->getDocument();
    doc->lock();
    bool redoResult = redoAction.redo(this->control);
    doc->unlock();

    if (!redoResult) {
        string msg = FS(_F("Could not redo \"{1}\"\n"
                           "Something went wrong… Please write a bug report…") %
                        redoAction.getText());
        XojMsgBox::showErrorToUser(control->getGtkWindow(), msg);
    }

    fireUpdateUndoRedoButtons(redoAction.getPages());

    printContents();
}

auto UndoRedoHandler::canUndo() -> bool { return !this->undoList.empty(); }

auto UndoRedoHandler::canRedo() -> bool { return !this->redoList.empty(); }

/**
 * Adds an undo Action to the list, or if nullptr does nothing
 */
void UndoRedoHandler::addUndoAction(UndoActionPtr action) {
    if (!action) {
        return;
    }

    this->undoList.emplace_back(std::move(action));
    clearRedo();
    fireUpdateUndoRedoButtons(this->undoList.back()->getPages());

    printContents();
}

auto UndoRedoHandler::undoDescription() -> string {
    if (!this->undoList.empty()) {
        UndoAction& a = *this->undoList.back();
        if (!a.getText().empty()) {
            string txt = _("Undo: ");
            txt += a.getText();
            return txt;
        }
    }
    return _("Undo");
}

auto UndoRedoHandler::redoDescription() -> string {
    if (!this->redoList.empty()) {
        UndoAction& a = *this->redoList.back();
        if (!a.getText().empty()) {
            string txt = _("Redo: ");
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
    if (this->undoList.empty()) {
        return this->savedUndo;
    }

    return this->savedUndo != this->undoList.back().get();
}

auto UndoRedoHandler::isChangedAutosave() -> bool {
    if (this->undoList.empty()) {
        return this->autosavedUndo;
    }
    return this->autosavedUndo != this->undoList.back().get();
}

void UndoRedoHandler::documentAutosaved() {
    this->autosavedUndo = this->undoList.empty() ? nullptr : this->undoList.back().get();
}

void UndoRedoHandler::documentSaved() {
    this->savedUndo = this->undoList.empty() ? nullptr : this->undoList.back().get();
}
