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

#include "XojPdfPage.h"
#include "XojPdfBookmarkIterator.h"

#include <XournalType.h>

#include <boost/filesystem/path.hpp>
using boost::filesystem::path;


class XojPdfDocumentInterface
{
public:
	XojPdfDocumentInterface();
	virtual ~XojPdfDocumentInterface();

public:
	virtual void assign(XojPdfDocumentInterface* doc) = 0;
	virtual bool equals(XojPdfDocumentInterface* doc) = 0;

public:
	virtual bool save(path filename, GError** error) = 0;
	virtual bool load(path filename, string password, GError** error) = 0;
	virtual bool isLoaded() = 0;

	virtual XojPdfPageSPtr getPage(size_t page) = 0;
	virtual size_t getPageCount() = 0;
	virtual XojPdfBookmarkIterator* getContentsIter() = 0;

private:
	XOJ_TYPE_ATTRIB;
};

