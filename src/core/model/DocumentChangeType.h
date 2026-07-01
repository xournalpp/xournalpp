/*
 * Xournal++
 *
 * Enum which type of change is occurred
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

enum DocumentChangeType {
    DOCUMENT_CHANGE_CLEARED,
    DOCUMENT_CHANGE_COMPLETE,
    DOCUMENT_CHANGE_PDF_BOOKMARKS,
    /// The loaded PDF content changed and PDF-backed views/caches should refresh.
    DOCUMENT_CHANGE_PDF_CONTENT
};
