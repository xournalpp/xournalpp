/*
 * Xournal++
 *
 * Undo action for moving page
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>  // for string

#include "control/Control.h"                    // for Control
#include "model/PageRef.h"                      // for PageRef
#include "undo/InsertDeletePageUndoAction.h"    // for InsertDeletePageUndoAction
#include "UndoAction.h"                         // for UndoAction

class Control;


class MovePageUndoAction: public UndoAction {
public:
    MovePageUndoAction(const PageRef& page, size_t oldPos, size_t newPos);
    ~MovePageUndoAction() override;

public:
    bool undo(Control* control) override;
    bool redo(Control* control) override;

    std::string getText() override;

private:
    InsertDeletePageUndoAction oldPos; // page delete at old position
    InsertDeletePageUndoAction newPos; // page insert at new position
};
