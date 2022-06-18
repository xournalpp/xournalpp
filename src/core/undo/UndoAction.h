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

#include <memory>  // for unique_ptr
#include <string>  // for string
#include <vector>  // for vector

#include "model/PageRef.h"  // for PageRef

class Control;

class UndoAction {
public:
    UndoAction(std::string className);  // NOLINT
    virtual ~UndoAction() = default;

public:
    virtual bool undo(Control* control) = 0;
    virtual bool redo(Control* control) = 0;

    virtual std::string getText() = 0;

    /**
     * Get the affected pages
     */
    virtual std::vector<PageRef> getPages();

    auto getClassName() const -> std::string const&;

protected:
    // This is only for debugging / Testing purpose
    std::string className;
    PageRef page;
    bool undone = false;
};

using UndoActionPtr = std::unique_ptr<UndoAction>;
