#include "InsertDeletePageUndoAction.h"

#include "control/Control.h"        // for Control
#include "control/ScrollHandler.h"  // for ScrollHandler
#include "gui/XournalppCursor.h"    // for XournalppCursor
#include "model/Document.h"         // for Document
#include "model/PageRef.h"          // for PageRef
#include "undo/UndoAction.h"        // for UndoAction
#include "util/Util.h"              // for npos
#include "util/i18n.h"              // for _

InsertDeletePageUndoAction::InsertDeletePageUndoAction(const PageRef& page, int pagePos, bool inserted):
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

    // see deletePage for why this is done
    // doc->lock();
    doc->insertPage(this->page, this->pagePos);
    doc->unlock();

    // these are all threadsafe (I think...)
    control->firePageInserted(this->pagePos);
    control->getCursor()->updateCursor();
    control->getScrollHandler()->scrollToPage(this->pagePos);
    control->updateDeletePageButton();

    // this prevents the double unlock
    doc->lock();
    return true;
}

auto InsertDeletePageUndoAction::deletePage(Control* control) -> bool {
    Document* doc = control->getDocument();

    // in order to fix the hang, we need to get out
    // of text mode
    //***This might kill whatever we've got selected
    control->clearSelectionEndText();

    // this was a double lock problem. We can fix it by
    // unlocking the UndoRedoHandler's lock inside here.
    // It's not great practise but it works.
    // doc->lock();
    auto pNr = doc->indexOf(page);
    if (pNr == npos) {
        //	doc->unlock();
        // this should not happen
        return false;
    }

    // first send event, then delete page...
    // we need to unlock the document from UndoRedoHandler
    // because firePageDeleted is threadsafe.
    doc->unlock();
    control->firePageDeleted(pNr);
    doc->lock();
    doc->deletePage(pNr);

    control->updateDeletePageButton();

    return true;
}

auto InsertDeletePageUndoAction::getText() -> std::string {
    if (this->inserted) {
        return _("Page inserted");
    }


    return _("Page deleted");
}
