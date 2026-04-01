#include "GroupUndoAction.h"

#include <algorithm>  // for none_of
#include <utility>    // for move

#include "undo/UndoAction.h"  // for UndoAction

class Control;

GroupUndoAction::GroupUndoAction(): UndoAction("GroupUndoAction") {}

void GroupUndoAction::addAction(std::unique_ptr<UndoAction> action) { actions.push_back(std::move(action)); }

auto GroupUndoAction::getPages() -> std::vector<PageRef> {
    std::vector<PageRef> pages;

    for (std::unique_ptr<UndoAction>& a: actions) {
        for (PageRef addPage: a->getPages()) {
            if (!addPage) {
                continue;
            }

            if (std::none_of(pages.begin(), pages.end(), [&](const PageRef& p) { return addPage == p; })) {
                pages.push_back(addPage);
            }
        }
    }

    return pages;
}

auto GroupUndoAction::redo(Control* control) -> bool {
    bool result = true;
    for (auto& action: actions) { result = result && action->redo(control); }

    return result;
}

auto GroupUndoAction::undo(Control* control) -> bool {
    bool result = true;
    for (auto& action: actions) { result = result && action->undo(control); }

    return result;
}

auto GroupUndoAction::getText() -> std::string {
    if (actions.empty()) {
        return "!! NOTHING !!";
    }

    return actions[0]->getText();
}
