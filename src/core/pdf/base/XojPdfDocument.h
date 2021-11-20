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


class XojPdfDocument: XojPdfDocumentInterface {
public:
    XojPdfDocument();
    XojPdfDocument(const XojPdfDocument& doc);
    virtual ~XojPdfDocument();

public:
    XojPdfDocument& operator=(const XojPdfDocument& doc);
    bool operator==(XojPdfDocument& doc);
    void assign(XojPdfDocumentInterface* doc) override;
    bool equals(XojPdfDocumentInterface* doc) override;

public:
    bool save(fs::path const& file, GError** error) override;
    bool load(fs::path const& file, std::string password, GError** error) override;
    bool load(GBytes* bytes, std::string password, GError** error) override;
    bool isLoaded() override;

    XojPdfPageSPtr getPage(size_t page) override;
    size_t getPageCount() override;
    XojPdfBookmarkIterator* getContentsIter() override;

private:
    XojPdfDocumentInterface* doc;
};
