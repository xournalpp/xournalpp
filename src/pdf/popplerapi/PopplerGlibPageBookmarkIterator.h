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

#include <poppler.h>

#include "pdf/base/XojPdfBookmarkIterator.h"

#include "PopplerGlibAction.h"
#include "XournalType.h"


class PopplerGlibPageBookmarkIterator: public XojPdfBookmarkIterator {
public:
    PopplerGlibPageBookmarkIterator(PopplerIndexIter* iter, PopplerDocument* document);
    virtual ~PopplerGlibPageBookmarkIterator();

public:
    virtual bool next();
    virtual bool isOpen();
    virtual XojPdfBookmarkIterator* getChildIter();
    virtual XojPdfAction* getAction();

private:
    PopplerIndexIter* iter;
    PopplerDocument* document;
};
