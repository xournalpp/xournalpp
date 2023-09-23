#include "MovePageUndoAction.h"

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
