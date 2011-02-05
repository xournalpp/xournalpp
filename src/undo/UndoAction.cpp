#include "UndoAction.h"

UndoAction::UndoAction() {
	this->undone = false;
}

XojPage ** UndoAction::getPages() {
	XojPage ** pages = new XojPage *[2];
	pages[0] = this->page;
	pages[1] = NULL;
	return pages;
}

