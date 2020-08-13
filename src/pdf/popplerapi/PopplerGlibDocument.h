/*
 * Xournal++
 *
 * Poppler GLib Implementation
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "filesystem.h"

#include <poppler.h>

#include "pdf/base/XojPdfDocumentInterface.h"

class PopplerGlibDocument: public XojPdfDocumentInterface {
public:
    PopplerGlibDocument();
    PopplerGlibDocument(const PopplerGlibDocument& doc);
    virtual ~PopplerGlibDocument();

public:
    virtual void assign(XojPdfDocumentInterface* doc);
    virtual bool equals(XojPdfDocumentInterface* doc);

public:
    virtual bool save(fs::path filename, GError** error);
    virtual bool load(fs::path filename, string password, GError** error);
    virtual bool load(gpointer data, gsize length, string password, GError** error);
    virtual bool isLoaded();

    virtual XojPdfPageSPtr getPage(size_t page);
    virtual size_t getPageCount();
    virtual XojPdfBookmarkIterator* getContentsIter();

private:
    PopplerDocument* document = nullptr;
};
