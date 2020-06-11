#include "GroupUndoAction.h"

GroupUndoAction::GroupUndoAction(): UndoAction("GroupUndoAction") {}

GroupUndoAction::~GroupUndoAction() {
    for (int i = actions.size() - 1; i >= 0; i--) {
        delete actions[i];
    }

    actions.clear();
}

void GroupUndoAction::addAction(UndoAction* action) { actions.push_back(action); }

auto GroupUndoAction::getPages() -> vector<PageRef> {
    vector<PageRef> pages;

    for (UndoAction* a: actions) {
        for (PageRef addPage: a->getPages()) {
            if (!addPage) {
                continue;
            }

            bool pageAlreadyInTheList = false;
            for (const PageRef& p: pages) {
                if (addPage == p) {
                    pageAlreadyInTheList = true;
                    break;
                }
            }

            if (!pageAlreadyInTheList) {
                pages.push_back(addPage);
            }
        }
    }

    return pages;
}

auto GroupUndoAction::redo(Control* control) -> bool {
    bool result = true;
    for (auto& action: actions) {
        result = result && action->redo(control);
    }

    return result;
}

auto GroupUndoAction::undo(Control* control) -> bool {
    bool result = true;
    for (int i = actions.size() - 1; i >= 0; i--) {
        result = result && actions[i]->undo(control);
    }

    return result;
}

auto GroupUndoAction::getText() -> string {
    if (actions.empty()) {
        return "!! NOTHING !!";
    }

    return actions[0]->getText();
}
