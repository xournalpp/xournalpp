#include "UndoAction.h"

#include <Rectangle.h>

UndoAction::UndoAction(const char* className)
 : className(className)
{
}

UndoAction::~UndoAction() = default;

auto UndoAction::getPages() -> vector<PageRef>
{
	vector<PageRef> pages;
	pages.push_back(this->page);
	return pages;
}

auto UndoAction::getClassName() const -> const char*
{
	return this->className;
}
