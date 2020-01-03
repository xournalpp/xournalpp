/*
 * Xournal++
 *
 * PDF Bookmark iterator interface
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>
#include <vector>

#include "XojPdfAction.h"
#include "XournalType.h"

class XojPdfBookmarkIterator {
public:
    XojPdfBookmarkIterator();
    virtual ~XojPdfBookmarkIterator();

public:
    virtual bool next() = 0;
    virtual bool isOpen() = 0;
    virtual XojPdfBookmarkIterator* getChildIter() = 0;
    virtual XojPdfAction* getAction() = 0;

private:
};
