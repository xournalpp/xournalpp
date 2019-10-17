#include "CopyUndoAction.h"

#include "control/Control.h"
#include "gui/XournalppCursor.h"
#include "model/PageRef.h"

#include <i18n.h>

CopyUndoAction::CopyUndoAction(PageRef pageref, int pageNr)
 : UndoAction("CopyUndoAction")
{
	this->page = pageref;
	this->pageNr = pageNr;
}

CopyUndoAction::~CopyUndoAction()
{
	this->page = nullptr;
}

bool CopyUndoAction::undo(Control* control)
{
	Document* doc = control->getDocument();

	// in order to fix the hang, we need to get out
	// of text mode
	// ***This might kill whatever we've got selected
	control->clearSelectionEndText();

	// first send event, then delete page...
	// we need to unlock the document from UndoRedoHandler
	// because firePageDeleted is threadsafe.
	doc->unlock();
	control->firePageDeleted(pageNr);
	doc->lock();
	doc->deletePage(pageNr);

	control->updateDeletePageButton();

	return true;
}

bool CopyUndoAction::redo(Control* control)
{
	Document* doc = control->getDocument();

	// just in case there would be a hang here,
	// we'll clear the selection in redo as well
	control->clearSelectionEndText();

	// see deletePage for why this is done
	// doc->lock();
	doc->insertPage(this->page, this->pageNr);
	doc->unlock();

	// these are all threadsafe (I think...)
	control->firePageInserted(this->pageNr);
	control->getCursor()->updateCursor();
	control->getScrollHandler()->scrollToPage(this->pageNr);
	control->updateDeletePageButton();

	// this prevents the double unlock
	doc->lock();
	return true;
}

string CopyUndoAction::getText()
{
	return _("Copy page");
}
