/*
 * Xournal++
 *
 * Document handler
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cstddef>  // for size_t
#include <list>     // for list

#include "DocumentChangeType.h"  // for DocumentChangeType

class DocumentListener;

class DocumentHandler {
public:
    DocumentHandler() = default;

    ~DocumentHandler() = default;

public:
    void fireDocumentChanged(DocumentChangeType type);
    void firePageSizeChanged(size_t page);
    void firePageChanged(size_t page);
    void firePageInserted(size_t page);
    void firePageDeleted(size_t page);
    // void firePageLoaded(PageRef page);
    void firePageSelected(size_t page);

private:
    void addListener(DocumentListener* l);
    void removeListener(DocumentListener* l);

private:
    std::list<DocumentListener*> listener;

    friend class DocumentListener;
};
