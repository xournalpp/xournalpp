#include "InsertDeletePageUndoAction.h"

#include "control/Control.h"        // for Control
#include "control/ScrollHandler.h"  // for ScrollHandler
#include "gui/XournalppCursor.h"    // for XournalppCursor
#include "model/Document.h"         // for Document
#include "model/PageRef.h"          // for PageRef
#include "undo/UndoAction.h"        // for UndoAction
#include "util/Util.h"              // for npos
#include "util/i18n.h"              // for _

InsertDeletePageUndoAction::InsertDeletePageUndoAction(const PageRef& page, size_t pagePos, bool inserted):
        UndoAction("InsertDeletePageUndoAction") {
    this->inserted = inserted;
    this->page = page;
    this->pagePos = pagePos;
}

InsertDeletePageUndoAction::~InsertDeletePageUndoAction() { this->page = nullptr; }

auto InsertDeletePageUndoAction::undo(Control* control) -> bool {
    if (this->inserted) {
        return deletePage(control);
    }


    return insertPage(control);
}

auto InsertDeletePageUndoAction::redo(Control* control) -> bool {
    if (this->inserted) {
        return insertPage(control);
    }


    return deletePage(control);
}

auto InsertDeletePageUndoAction::insertPage(Control* control) -> bool {
    Document* doc = control->getDocument();

    // just in case there would be a hang here,
    // we'll clear the selection in redo as well
    control->clearSelectionEndText();

    doc->lock();
    doc->insertPage(this->page, this->pagePos);
    doc->unlock();

    control->firePageInserted(this->pagePos);
    control->getCursor()->updateCursor();
    control->getScrollHandler()->scrollToPage(this->pagePos);

    return true;
}

auto InsertDeletePageUndoAction::deletePage(Control* control) -> bool {
    Document* doc = control->getDocument();

    // in order to fix the hang, we need to get out
    // of text mode
    //***This might kill whatever we've got selected
    control->clearSelectionEndText();

    doc->lock();
    auto pNr = doc->indexOf(page);
    doc->unlock();
    if (pNr == npos) {
        // this should not happen
        return false;
    }

    // first send event, then delete page...
    control->firePageDeleted(pNr);
    doc->lock();
    doc->deletePage(pNr);
    doc->unlock();
    return true;
}

auto InsertDeletePageUndoAction::getText() -> std::string {
    if (this->inserted) {
        return _("Page inserted");
    }


    return _("Page deleted");
}
