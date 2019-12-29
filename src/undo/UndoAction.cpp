#include "UndoAction.h"

#include "Rectangle.h"

UndoAction::UndoAction(std::string className): className(std::move(className)) {}

auto UndoAction::getPages() -> std::vector<PageRef> {
    std::vector<PageRef> pages;
    pages.push_back(this->page);
    return pages;
}

auto UndoAction::getClassName() const -> std::string const& { return this->className; }
