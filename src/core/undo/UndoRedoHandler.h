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

#include <deque>   // for deque
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

    /*
        When you write something the page must be inserted here
    */
    static std::deque<int> pagesChanged; 
    
    /*
        If you do undo you should insert the page in here and in case of redo go back to pagesChanged
    */
    
    static std::deque<int> pagesChangedUndo;

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
