#include "SwapUndoAction.h"
#include "model/Document.h"
#include "control/Control.h"

SwapUndoAction::SwapUndoAction(int pageNr,
                               bool moveUp,
                               PageRef swapped_page,
                               PageRef other_page)
	: UndoAction("SwapUndoAction")
{
	XOJ_INIT_TYPE(SwapUndoAction);

	this->pageNr = pageNr;
	this->moveUp = moveUp;
	this->swapped_page = swapped_page;
	this->other_page = other_page;
}

SwapUndoAction::~SwapUndoAction()
{
	XOJ_CHECK_TYPE(SwapUndoAction);

	XOJ_RELEASE_TYPE(SwapUndoAction);
}

bool SwapUndoAction::undo(Control* control)
{
	XOJ_CHECK_TYPE(SwapUndoAction);

	swap(control);
	this->undone = true;

	return true;
}

bool SwapUndoAction::redo(Control* control)
{
	XOJ_CHECK_TYPE(SwapUndoAction);

	swap(control);
	this->undone = false;

	return true;
}

void SwapUndoAction::swap(Control* control)
{
	XOJ_CHECK_TYPE(SwapUndoAction);

	Document* doc = control->getDocument();

	doc->unlock();

	gint insertPos = this->pageNr,
	     deletePos = this->pageNr + 1;

	if(moveUp != this->undone)
	{
		std::swap(insertPos, deletePos);
	}

	doc->deletePage(deletePos);
	doc->insertPage(this->swapped_page, insertPos);

	control->firePageDeleted(deletePos);
	control->firePageInserted(insertPos);
	control->firePageSelected(insertPos);

	control->getScrollHandler()->scrollToPage(insertPos);

	doc->lock();
}

XojPage** SwapUndoAction::getPages()
{
	XOJ_CHECK_TYPE(SwapUndoAction);

	XojPage** pages = new XojPage *[3];
	pages[0] = this->swapped_page;
	pages[1] = this->other_page;
	pages[2] = NULL;
	return pages;
}

string SwapUndoAction::getText()
{
	XOJ_CHECK_TYPE(SwapUndoAction);

	return moveUp ? _("Move page upwards") : _("Move page downwards");
}
