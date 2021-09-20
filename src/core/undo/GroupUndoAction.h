/*
 * Xournal++
 *
 * Undo action to group a list of undo actions of the same type
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>
#include <vector>

#include "UndoAction.h"


class GroupUndoAction: public UndoAction {
public:
    GroupUndoAction();

public:
    void addAction(std::unique_ptr<UndoAction> action);

    /**
     * Get the affected pages
     */
    std::vector<PageRef> getPages() override;

    bool undo(Control* control) override;
    bool redo(Control* control) override;

    std::string getText() override;

private:
    std::vector<std::unique_ptr<UndoAction>> actions;
};
