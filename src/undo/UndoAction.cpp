#include "UndoAction.h"

UndoAction::UndoAction() {
	XOJ_INIT_TYPE(UndoAction);

	this->undone = false;
}

XojPage ** UndoAction::getPages() {
	XOJ_CHECK_TYPE(UndoAction);

	XojPage ** pages = new XojPage *[2];
	pages[0] = this->page;
	pages[1] = NULL;
	return pages;

	XOJ_RELEASE_TYPE(UndoAction);
}

