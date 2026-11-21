/*
 * Xournal++
 *
 * Handles Undo and Redo
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>  // for string
#include <vector>  // for vector

#include "model/PageRef.h"  // for PageRef

#include "UndoAction.h"  // for UndoActionPtr

class Control;

class UndoRedoListener {
public:
    virtual void undoRedoChanged() = 0;
    virtual void undoRedoPageChanged(PageRef page) = 0;

    virtual ~UndoRedoListener() = default;
};

class UndoRedoHandler {
public:
    explicit UndoRedoHandler(Control* control);
    virtual ~UndoRedoHandler();

    void undo();
    void redo();

    bool canUndo();
    bool canRedo();

    void addUndoAction(UndoActionPtr action);

    std::string undoDescription();
    std::string redoDescription();

    void clearContents();

    void fireUpdateUndoRedoButtons(const std::vector<PageRef>& pages);
    void addUndoRedoListener(UndoRedoListener* listener);

    bool isChanged();
    bool isChangedAutosave();
    void documentAutosaved();
    void documentSaved();

private:
    void clearRedo();
    void printContents();

private:
    /// A circular buffer, with an extra head indicating undone operations
    struct UndoStack {
        UndoStack();

    private:
        std::vector<UndoActionPtr> operations;

    public:
        using iterator = decltype(operations)::iterator;

        // the undo part is [start, head), the redo part is [head, end)
        iterator start;  ///< oldest operation in the stack. Will get overriden upon overflow
        iterator head;   ///< either end or first undone operation. == next(most recent not-undone operation)
        iterator end;    ///< past the last operation (done or undone alike)

        iterator next(iterator it);  ///< next, going back to operations.begin() if end() is reached
        iterator prev(iterator it);  ///< prev, going back to std::prev(operations.end()) if at begin()
    };
    UndoStack stack;

    UndoAction* savedUndo = nullptr;
    UndoAction* autosavedUndo = nullptr;

    std::vector<UndoRedoListener*> listener;

    Control* control = nullptr;
};
