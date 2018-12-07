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

#include "pdf/popplerdirect/poppler/XojPopplerDocument.h"

#include <XournalType.h>

#include <glib.h>

class XojPdfDocument
{
public:
	XojPdfDocument();
	XojPdfDocument(const XojPdfDocument& doc);
	virtual ~XojPdfDocument();

public:
	void operator=(XojPdfDocument& doc);
	bool operator==(XojPdfDocument& doc);

public:
	bool save(path filename, GError** error);
	bool load(path filename, string password, GError** error);
	bool isLoaded();

	XojPdfPage* getPage(size_t page);
	size_t getPageCount();
	XojPdfBookmarkIterator* getContentsIter();

public:
	// This method will later be enabled / disabled by precompiler macros
	XojPopplerDocument& getPopplerDocument();

private:
	XOJ_TYPE_ATTRIB;

	XojPopplerDocument doc;
};

