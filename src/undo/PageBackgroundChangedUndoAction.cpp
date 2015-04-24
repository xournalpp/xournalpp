#include "PageBackgroundChangedUndoAction.h"
#include "control/Control.h"
#include "model/Document.h"

PageBackgroundChangedUndoAction::PageBackgroundChangedUndoAction(PageRef page,
			BackgroundType origType,
			int origPdfPage, BackgroundImage origBackgroundImage, double origW,
			double origH)
	: UndoAction("PageBackgroundChangedUndoAction")
{
	XOJ_INIT_TYPE(PageBackgroundChangedUndoAction);

	this->page = page;
	this->origType = origType;
	this->origPdfPage = this->origPdfPage;
	this->origBackgroundImage = origBackgroundImage;
	this->origW = origW;
	this->origH = origH;
}

PageBackgroundChangedUndoAction::~PageBackgroundChangedUndoAction()
{
	XOJ_RELEASE_TYPE(PageBackgroundChangedUndoAction);
}

bool PageBackgroundChangedUndoAction::undo(Control* control)
{
	XOJ_CHECK_TYPE(PageBackgroundChangedUndoAction);

	this->newType = this->page->getBackgroundType();
	this->newPdfPage = this->page->getPdfPageNr();
	this->newBackgroundImage = this->page->getBackgroundImage();
	this->newW = this->page->getWidth();
	this->newH = this->page->getHeight();

	Document* doc = control->getDocument();
	int pageNr = doc->indexOf(this->page);
	if (pageNr == -1)
	{
		return false;
	}

	if (this->newW != this->origW || this->newH != this->origH)
	{
		this->page->setSize(this->origW, this->origH);
		control->firePageSizeChanged(pageNr);
	}

	this->page->setBackgroundType(this->origType);
	if (this->origType == BACKGROUND_TYPE_PDF)
	{
		this->page->setBackgroundPdfPageNr(this->origPdfPage);
	}
	else if (this->origType == BACKGROUND_TYPE_IMAGE)
	{
		this->page->setBackgroundImage(this->origBackgroundImage);
	}

	control->firePageChanged(pageNr);

	return true;
}

bool PageBackgroundChangedUndoAction::redo(Control* control)
{
	XOJ_CHECK_TYPE(PageBackgroundChangedUndoAction);

	Document* doc = control->getDocument();

	int pageNr = doc->indexOf(this->page);

	if (pageNr == -1)
	{
		return false;
	}

	if (this->newW != this->origW || this->newH != this->origH)
	{
		this->page->setSize(this->newW, this->newH);
		control->firePageSizeChanged(pageNr);
	}

	this->page->setBackgroundType(this->newType);
	if (this->newType == BACKGROUND_TYPE_PDF)
	{
		this->page->setBackgroundPdfPageNr(this->newPdfPage);
	}
	else if (this->newType == BACKGROUND_TYPE_IMAGE)
	{
		this->page->setBackgroundImage(this->newBackgroundImage);
	}

	control->firePageChanged(pageNr);

	return true;

}

string PageBackgroundChangedUndoAction::getText()
{
	XOJ_CHECK_TYPE(PageBackgroundChangedUndoAction);

	return _("Page background changed");
}
