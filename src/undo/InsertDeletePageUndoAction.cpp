#include "InsertDeletePageUndoAction.h"
#include "../model/Page.h"
#include "../model/Document.h"
#include "../control/Control.h"
#include "../gui/Cursor.h"

InsertDeletePageUndoAction::InsertDeletePageUndoAction(XojPage * page, int pagePos, bool inserted) {
	XOJ_INIT_TYPE(InsertDeletePageUndoAction);

	this->inserted = inserted;
	this->page = page;
	this->pagePos = pagePos;
	page->reference(16);
}

InsertDeletePageUndoAction::~InsertDeletePageUndoAction() {
	XOJ_CHECK_TYPE(InsertDeletePageUndoAction);

	this->page->unreference(17);
	this->page = NULL;

	XOJ_RELEASE_TYPE(InsertDeletePageUndoAction);
}

bool InsertDeletePageUndoAction::undo(Control * control) {
	XOJ_CHECK_TYPE(InsertDeletePageUndoAction);

	if (this->inserted) {
		return deletePage(control);
	} else {
		return insertPage(control);
	}
}

bool InsertDeletePageUndoAction::redo(Control * control) {
	XOJ_CHECK_TYPE(InsertDeletePageUndoAction);

	if (this->inserted) {
		return insertPage(control);
	} else {
		return deletePage(control);
	}
}

bool InsertDeletePageUndoAction::insertPage(Control * control) {
	XOJ_CHECK_TYPE(InsertDeletePageUndoAction);

	Document * doc = control->getDocument();

	doc->lock();
	doc->insertPage(this->page, this->pagePos);
	doc->unlock();

	control->firePageInserted(this->pagePos);
	control->getCursor()->updateCursor();
	control->getScrollHandler()->scrollToPage(this->pagePos);
	control->updateDeletePageButton();

	return true;
}

bool InsertDeletePageUndoAction::deletePage(Control * control) {
	XOJ_CHECK_TYPE(InsertDeletePageUndoAction);

	Document * doc = control->getDocument();

	doc->lock();
	int pNr = doc->indexOf(page);
	if (pNr == -1) {
		doc->unlock();
		// this should not happen
		return false;
	}

	// first send event, then delete page...
	control->firePageDeleted(pNr);
	doc->deletePage(pNr);

	control->updateDeletePageButton();

	doc->unlock();
	return true;
}

String InsertDeletePageUndoAction::getText() {
	XOJ_CHECK_TYPE(InsertDeletePageUndoAction);

	if (this->inserted) {
		return _("Page inserted");
	} else {
		return _("Page deleted");
	}
}
