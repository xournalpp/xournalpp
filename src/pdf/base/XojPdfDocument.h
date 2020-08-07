/*
 * Xournal++
 *
 * PDF Document Container
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>
#include <vector>

#include "pdf/base/XojPdfDocumentInterface.h"

#include "XojPdfBookmarkIterator.h"
#include "XojPdfDocumentInterface.h"
#include "XojPdfPage.h"
#include "XournalType.h"

class XojPdfDocument: XojPdfDocumentInterface {
public:
    XojPdfDocument();
    XojPdfDocument(const XojPdfDocument& doc);
    virtual ~XojPdfDocument();

public:
    XojPdfDocument& operator=(const XojPdfDocument& doc);
    bool operator==(XojPdfDocument& doc);
    void assign(XojPdfDocumentInterface* doc);
    bool equals(XojPdfDocumentInterface* doc);

public:
    bool save(fs::path const& file, GError** error);
    bool load(fs::path const& file, string password, GError** error);
    bool load(gpointer data, gsize length, string password, GError** error);
    bool isLoaded();

    XojPdfPageSPtr getPage(size_t page);
    size_t getPageCount();
    XojPdfBookmarkIterator* getContentsIter();

public:
    XojPdfDocumentInterface* getDocumentInterface();

private:
    XojPdfDocumentInterface* doc;
};
