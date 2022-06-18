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

#include <cstddef>  // for size_t
#include <string>   // for string

#include <glib.h>     // for GError, gpointer, gsize
#include <poppler.h>  // for PopplerDocument

#include "pdf/base/XojPdfDocumentInterface.h"  // for XojPdfDocumentInterface
#include "pdf/base/XojPdfPage.h"               // for XojPdfPageSPtr

#include "filesystem.h"  // for path

class XojPdfBookmarkIterator;

class PopplerGlibDocument: public XojPdfDocumentInterface {
public:
    PopplerGlibDocument();
    PopplerGlibDocument(const PopplerGlibDocument& doc);
    ~PopplerGlibDocument() override;

public:
    void assign(XojPdfDocumentInterface* doc) override;
    bool equals(XojPdfDocumentInterface* doc) const override;

public:
    bool save(fs::path const& filepath, GError** error) const override;
    bool load(fs::path const& filepath, std::string password, GError** error) override;
    bool load(gpointer data, gsize length, std::string password, GError** error) override;
    bool isLoaded() const override;

    XojPdfPageSPtr getPage(size_t page) const override;
    size_t getPageCount() const override;
    XojPdfBookmarkIterator* getContentsIter() const override;

private:
    PopplerDocument* document = nullptr;
};
