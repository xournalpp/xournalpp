#include "UndoAction.h"

UndoAction::UndoAction(const char * className) : className(className) {
	XOJ_INIT_TYPE(UndoAction);

	this->undone = false;
}

UndoAction::~UndoAction() {
	XOJ_RELEASE_TYPE(UndoAction);
}

XojPage ** UndoAction::getPages() {
	XOJ_CHECK_TYPE(UndoAction);

	XojPage ** pages = new XojPage *[2];
	pages[0] = this->page;
	pages[1] = NULL;
	return pages;

	XOJ_RELEASE_TYPE(UndoAction);
}

const char * UndoAction::getClassName() const {
	return this->className;
}

