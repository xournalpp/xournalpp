#pragma once

#include <optional>
#include <string>

#include "undo/UndoAction.h"

class Control;

class BookmarkUndoAction: public UndoAction {
public:
    BookmarkUndoAction(size_t pageIndex,
                       std::optional<std::string> oldBookmark,
                       std::optional<std::string> newBookmark);

    ~BookmarkUndoAction() override = default;

    auto getText() -> std::string override;
    auto undo(Control* control) -> bool override;
    auto redo(Control* control) -> bool override;

private:
    size_t pageIndex;
    std::optional<std::string> oldBookmark;
    std::optional<std::string> newBookmark;

    auto apply(Control* control, const std::optional<std::string>& bm1) -> bool;
};
