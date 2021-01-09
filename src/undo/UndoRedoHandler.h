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

#include <deque>
#include <memory>
#include <stack>
#include <string>
#include <vector>

#include "UndoAction.h"
#include "XournalType.h"

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
    void addUndoActionBefore(UndoActionPtr action, UndoAction* before);
    bool removeUndoAction(UndoAction* action);

    string undoDescription();
    string redoDescription();

    void clearContents();

    void fireUpdateUndoRedoButtons(const vector<PageRef>& pages);
    void addUndoRedoListener(UndoRedoListener* listener);

    bool isChanged();
    bool isChangedAutosave();
    void documentAutosaved();
    void documentSaved();

private:
    void clearRedo();
    void printContents();

private:
    std::deque<UndoActionPtr> undoList;
    std::deque<UndoActionPtr> redoList;

    UndoAction* savedUndo = nullptr;
    UndoAction* autosavedUndo = nullptr;

    std::vector<UndoRedoListener*> listener;

    Control* control = nullptr;
};
