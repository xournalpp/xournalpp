/*
 * Xournal++
 *
 * Abstract undo action
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "model/PageRef.h"

#include "config.h"

class Control;
class XojPage;

class UndoAction {
public:
    UndoAction(std::string className);  // NOLINT
    virtual ~UndoAction() = default;

public:
    virtual bool undo(Control* control) = 0;
    virtual bool redo(Control* control) = 0;

    virtual string getText() = 0;

    /**
     * Get the affected pages
     */
    virtual vector<PageRef> getPages();

    auto getClassName() const -> std::string const&;

protected:
    // This is only for debugging / Testing purpose
    std::string className;
    PageRef page;
    bool undone = false;
};

using UndoActionPtr = std::unique_ptr<UndoAction>;
