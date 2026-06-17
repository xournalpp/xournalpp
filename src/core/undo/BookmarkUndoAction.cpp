#include "BookmarkUndoAction.h"

#include "control/Control.h"
#include "model/Document.h"
#include "model/DocumentChangeType.h"
#include "model/XojPage.h"
#include "util/i18n.h"

BookmarkUndoAction::BookmarkUndoAction(size_t pageIndex, std::optional<std::string> oldBookmark,
                                       std::optional<std::string> newBookmark):
        UndoAction("BookmarkUndoAction"),
        pageIndex(pageIndex),
        oldBookmark(std::move(oldBookmark)),
        newBookmark(std::move(newBookmark)) {}

auto BookmarkUndoAction::getText() -> std::string {
    if (!oldBookmark.has_value())
        return _("Add bookmark");
    if (!newBookmark.has_value())
        return _("Delete bookmark");
    return _("Rename bookmark");
}

auto BookmarkUndoAction::undo(Control* control) -> bool { return apply(control, oldBookmark); }

auto BookmarkUndoAction::redo(Control* control) -> bool { return apply(control, newBookmark); }

auto BookmarkUndoAction::apply(Control* control, const std::optional<std::string>& bm) -> bool {

    Document* doc = control->getDocument();

    doc->lock();
    PageRef page = doc->getPage(pageIndex);
    DocumentChangeType docChangeType = DOCUMENT_CHANGE_BOOKMARKS;

    if (bm.has_value()) {
        page->setBookmark(bm.value());
    } else {
        page->deleteBookmark();
        if (doc->listBookmarks().empty()) {
            docChangeType = DOCUMENT_CHANGE_NO_BOOKMARKS;  // To force the entire sidebar to update
        }
    }
    doc->unlock();

    control->getUndoRedoHandler()->fireUpdateUndoRedoButtons({page});
    control->fireDocumentChanged(docChangeType);

    return true;
}
