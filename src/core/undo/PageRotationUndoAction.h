/*
 * Xournal++
 *
 * Undo action for page rotation
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>  // for string

#include "model/PageRef.h"  // for PageRef
#include "model/XojPage.h"  // for PageOrientation

#include "UndoAction.h"  // for UndoAction

class Control;


class PageRotationUndoAction: public UndoAction {
public:
    PageRotationUndoAction(const PageRef& page, PageOrientation oldPgOrientation, PageOrientation newPgOrientation);
    ~PageRotationUndoAction() override;

public:
    bool undo(Control* control) override;
    bool redo(Control* control) override;

    std::string getText() override;

private:
    bool applyRotation(Control* ctrl, PageOrientation orientation);
    PageOrientation oldPgOrientation;
    PageOrientation newPgOrientation;
};
