/*
 * Xournal++
 *
 * PDF Document Container Interface
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>
#include <vector>

#include "XojPdfBookmarkIterator.h"
#include "XojPdfPage.h"
#include "XournalType.h"
#include "filesystem.h"

class XojPdfDocumentInterface {
public:
    XojPdfDocumentInterface();
    virtual ~XojPdfDocumentInterface();

public:
    virtual void assign(XojPdfDocumentInterface* doc) = 0;
    virtual bool equals(XojPdfDocumentInterface* doc) = 0;

public:
    virtual bool save(fs::path const& file, GError** error) = 0;
    virtual bool load(fs::path const& file, string password, GError** error) = 0;
    virtual bool load(gpointer data, gsize length, string password, GError** error) = 0;
    virtual bool isLoaded() = 0;

    virtual XojPdfPageSPtr getPage(size_t page) = 0;
    virtual size_t getPageCount() = 0;
    virtual XojPdfBookmarkIterator* getContentsIter() = 0;

private:
};
