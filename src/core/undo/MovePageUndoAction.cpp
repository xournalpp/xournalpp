#include "MovePageUndoAction.h"

#include "control/Control.h"        // for Control
#include "control/ScrollHandler.h"  // for ScrollHandler
#include "gui/XournalppCursor.h"    // for XournalppCursor
#include "model/Document.h"         // for Document
#include "model/PageRef.h"          // for PageRef
#include "undo/UndoAction.h"        // for UndoAction
#include "util/Util.h"              // for npos
#include "util/i18n.h"              // for _

MovePageUndoAction::MovePageUndoAction(const PageRef& page, size_t oldPos, size_t newPos):
        UndoAction("MovePageUndoAction"),
        oldPos(page, newPos, true),
        newPos(page, oldPos, false) {
    this->page = page;
}

MovePageUndoAction::~MovePageUndoAction() { this->page = nullptr; }

auto MovePageUndoAction::undo(Control* control) -> bool {
    bool oldRes = oldPos.undo(control);
    bool newRes = newPos.undo(control);
    return oldRes && newRes;
}

auto MovePageUndoAction::redo(Control* control) -> bool {
    bool newRes = newPos.redo(control);
    bool oldRes = oldPos.redo(control);
    return oldRes && newRes;
}

auto MovePageUndoAction::getText() -> std::string {
    return _("Page move");
}
