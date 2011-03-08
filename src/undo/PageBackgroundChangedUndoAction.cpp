#include "PageBackgroundChangedUndoAction.h"
#include "../control/Control.h"
#include "../model/Document.h"

PageBackgroundChangedUndoAction::PageBackgroundChangedUndoAction(XojPage * page, BackgroundType origType,
		int origPdfPage, BackgroundImage origBackgroundImage, double origW, double origH) {
	this->page = page;
	this->origType = origType;
	this->origPdfPage = this->origPdfPage;
	this->origBackgroundImage = origBackgroundImage;
	this->origW = origW;
	this->origH = origH;
}
PageBackgroundChangedUndoAction::~PageBackgroundChangedUndoAction() {
}

bool PageBackgroundChangedUndoAction::undo(Control * control) {
	this->newType = this->page->getBackgroundType();
	this->newPdfPage = this->page->getPdfPageNr();
	this->newBackgroundImage = this->page->backgroundImage;
	this->newW = this->page->getWidth();
	this->newH = this->page->getHeight();

	Document * doc = control->getDocument();
	doc->lock();

	int pageNr = doc->indexOf(this->page);
	if (pageNr == -1) {
		doc->unlock();
		return false;
	}

	if (this->newW != this->origW || this->newH != this->origH) {
		this->page->setSize(this->origW, this->origH);
		control->firePageSizeChanged(pageNr);
	}

	this->page->setBackgroundType(this->origType);
	if (this->origType == BACKGROUND_TYPE_PDF) {
		this->page->setBackgroundPdfPageNr(this->origPdfPage);
	} else if (this->origType == BACKGROUND_TYPE_IMAGE) {
		this->page->backgroundImage = this->origBackgroundImage;
	}

	doc->unlock();
	control->firePageChanged(pageNr);

	return true;
}

bool PageBackgroundChangedUndoAction::redo(Control * control) {
	Document * doc = control->getDocument();

	doc->lock();
	int pageNr = doc->indexOf(this->page);
	doc->unlock();

	if (pageNr == -1) {
		return false;
	}

	if (this->newW != this->origW || this->newH != this->origH) {
		this->page->setSize(this->newW, this->newH);
		control->firePageSizeChanged(pageNr);
	}

	this->page->setBackgroundType(this->newType);
	if (this->newType == BACKGROUND_TYPE_PDF) {
		this->page->setBackgroundPdfPageNr(this->newPdfPage);
	} else if (this->newType == BACKGROUND_TYPE_IMAGE) {
		this->page->backgroundImage = this->newBackgroundImage;
	}

	control->firePageChanged(pageNr);

	return true;

}

String PageBackgroundChangedUndoAction::getText() {
	return _("Page background changed");
}
