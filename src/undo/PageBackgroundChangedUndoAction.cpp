#include "PageBackgroundChangedUndoAction.h"

#include "control/Control.h"
#include "model/Document.h"

#include <i18n.h>

PageBackgroundChangedUndoAction::PageBackgroundChangedUndoAction(PageRef page, PageType origType, int origPdfPage,
																 BackgroundImage origBackgroundImage,
																 double origW, double origH)
 : UndoAction("PageBackgroundChangedUndoAction")
{
	XOJ_INIT_TYPE(PageBackgroundChangedUndoAction);

	this->page = page;
	this->origType = origType;
	this->origPdfPage = origPdfPage;
	this->origBackgroundImage = origBackgroundImage;
	this->origW = origW;
	this->origH = origH;
	this->newW = 0;
	this->newH = 0;
	this->newPdfPage = -1;
	this->newType.format = "plain";
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
	if (this->origType.isPdfPage())
	{
		this->page->setBackgroundPdfPageNr(this->origPdfPage);
	}
	else if (this->origType.isImagePage())
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
	if (this->newType.isPdfPage())
	{
		this->page->setBackgroundPdfPageNr(this->newPdfPage);
	}
	else if (this->newType.isImagePage())
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
