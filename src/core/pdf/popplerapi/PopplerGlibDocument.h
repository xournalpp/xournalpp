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

#include <poppler.h>

#include "pdf/base/XojPdfDocumentInterface.h"

#include "filesystem.h"

class PopplerGlibDocument: public XojPdfDocumentInterface {
public:
    PopplerGlibDocument();
    PopplerGlibDocument(const PopplerGlibDocument& doc);
    ~PopplerGlibDocument() override;

public:
    void assign(XojPdfDocumentInterface* doc) override;
    bool equals(XojPdfDocumentInterface* doc) override;

public:
    bool save(fs::path const& filepath, GError** error) override;
    bool load(fs::path const& filepath, std::string password, GError** error) override;
    bool load(gpointer data, gsize length, std::string password, GError** error) override;
    bool isLoaded() override;

    XojPdfPageSPtr getPage(size_t page) override;
    size_t getPageCount() override;
    XojPdfBookmarkIterator* getContentsIter() override;

private:
    PopplerDocument* document = nullptr;
};
