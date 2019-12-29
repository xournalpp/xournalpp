#include "SwapUndoAction.h"

#include "control/Control.h"
#include "model/Document.h"

#include "i18n.h"

SwapUndoAction::SwapUndoAction(size_t pageNr, bool moveUp, const PageRef& swappedPage, const PageRef& otherPage):
        UndoAction("SwapUndoAction") {
    this->pageNr = pageNr;
    this->moveUp = moveUp;
    this->swappedPage = swappedPage;
    this->otherPage = otherPage;
}

SwapUndoAction::~SwapUndoAction() = default;

auto SwapUndoAction::undo(Control* control) -> bool {
    swap(control);
    this->undone = true;

    return true;
}

auto SwapUndoAction::redo(Control* control) -> bool {
    swap(control);
    this->undone = false;

    return true;
}

void SwapUndoAction::swap(Control* control) {
    Document* doc = control->getDocument();

    doc->unlock();

    gint insertPos = this->pageNr;
    gint deletePos = this->pageNr + 1;

    if (moveUp != this->undone) {
        std::swap(insertPos, deletePos);
    }

    doc->deletePage(deletePos);
    doc->insertPage(this->swappedPage, insertPos);

    control->firePageDeleted(deletePos);
    control->firePageInserted(insertPos);
    control->firePageSelected(insertPos);

    control->getScrollHandler()->scrollToPage(insertPos);

    doc->lock();
}

auto SwapUndoAction::getPages() -> vector<PageRef> {
    vector<PageRef> pages;
    pages.push_back(this->swappedPage);
    pages.push_back(this->otherPage);
    return pages;
}

auto SwapUndoAction::getText() -> string { return moveUp ? _("Move page upwards") : _("Move page downwards"); }
