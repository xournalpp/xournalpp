/*
 * Xournal++
 *
 * Undo action for page copy
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>  // for string

#include "model/PageRef.h"  // for PageRef

#include "UndoAction.h"  // for UndoAction

class Control;


class CopyUndoAction: public UndoAction {
public:
    CopyUndoAction(const PageRef& pageref, int pageNr);
    ~CopyUndoAction() override;

public:
    bool undo(Control* control) override;
    bool redo(Control* control) override;

    std::string getText() override;

private:
    int pageNr;
};
