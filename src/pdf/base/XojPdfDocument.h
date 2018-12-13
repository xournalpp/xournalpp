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

#include "XojPdfPage.h"
#include "XojPdfBookmarkIterator.h"
#include "XojPdfDocumentInterface.h"

#include "pdf/base/XojPdfDocumentInterface.h"

#include <XournalType.h>

#include <glib.h>

class XojPdfDocument : XojPdfDocumentInterface
{
public:
	XojPdfDocument();
	XojPdfDocument(const XojPdfDocument& doc);
	virtual ~XojPdfDocument();

public:
	void operator=(XojPdfDocument& doc);
	bool operator==(XojPdfDocument& doc);
	void assign(XojPdfDocumentInterface* doc);
	bool equals(XojPdfDocumentInterface* doc);

public:
	bool save(path filename, GError** error);
	bool load(path filename, string password, GError** error);
	bool isLoaded();

	XojPdfPageSPtr getPage(size_t page);
	size_t getPageCount();
	XojPdfBookmarkIterator* getContentsIter();

public:
	XojPdfDocumentInterface* getDocumentInterface();

private:
	XOJ_TYPE_ATTRIB;

	XojPdfDocumentInterface* doc;
};

