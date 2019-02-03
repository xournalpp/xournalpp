#include "UndoAction.h"

#include <Rectangle.h>

UndoAction::UndoAction(const char* className)
 : className(className)
{
	XOJ_INIT_TYPE(UndoAction);

	this->undone = false;
}

UndoAction::~UndoAction()
{
	XOJ_RELEASE_TYPE(UndoAction);
}

vector<PageRef> UndoAction::getPages()
{
	XOJ_CHECK_TYPE(UndoAction);

	vector<PageRef> pages;
	pages.push_back(this->page);
	return pages;
}

const char* UndoAction::getClassName() const
{
	return this->className;
}
