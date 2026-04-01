/*
 * Xournal++
 *
 * Document listener
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cstddef>  // for size_t

#include "DocumentChangeType.h"  // for DocumentChangeType

class DocumentHandler;

class DocumentListener {
public:
    DocumentListener();
    virtual ~DocumentListener();

public:
    void registerListener(DocumentHandler* handler);
    void unregisterListener();

    virtual void documentChanged(DocumentChangeType type);
    virtual void pageSizeChanged(size_t page);
    virtual void pageChanged(size_t page);
    virtual void pageInserted(size_t page);
    virtual void pageDeleted(size_t page);
    virtual void pageSelected(size_t page);

private:
    DocumentHandler* handler = nullptr;
};
