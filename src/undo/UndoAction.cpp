#include "UndoAction.h"

#include <Rectangle.h>

UndoAction::UndoAction(const char* className)
 : className(className)
{
}

UndoAction::~UndoAction()
{
}

vector<PageRef> UndoAction::getPages()
{
	vector<PageRef> pages;
	pages.push_back(this->page);
	return pages;
}

const char* UndoAction::getClassName() const
{
	return this->className;
}
