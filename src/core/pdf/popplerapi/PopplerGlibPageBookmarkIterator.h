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

#include <poppler.h>  // for PopplerDocument, Popple...

#include "pdf/base/XojPdfBookmarkIterator.h"  // for XojPdfBookmarkIterator

class XojPdfAction;


class PopplerGlibPageBookmarkIterator: public XojPdfBookmarkIterator {
public:
    PopplerGlibPageBookmarkIterator(PopplerIndexIter* iter, PopplerDocument* document);
    ~PopplerGlibPageBookmarkIterator() override;

public:
    bool next() override;
    bool isOpen() override;
    XojPdfBookmarkIterator* getChildIter() override;
    XojPdfAction* getAction() override;

private:
    PopplerIndexIter* iter;
    PopplerDocument* document;
};
