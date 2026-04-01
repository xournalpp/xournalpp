/*
 * Xournal++
 *
 * Undo action for page size change
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


class PageSizeChangeUndoAction: public UndoAction {
public:
    PageSizeChangeUndoAction(const PageRef& page, double origW, double origH);
    ~PageSizeChangeUndoAction() override;

public:
    bool undo(Control* control) override;
    bool redo(Control* control) override;

    std::string getText() override;

private:
    bool swapSizes(Control* ctrl);
    double otherWidth;
    double otherHeight;
};
