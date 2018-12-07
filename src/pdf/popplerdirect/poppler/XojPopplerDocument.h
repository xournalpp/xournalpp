/*
 * Xournal++
 *
 * Custom Poppler access library
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "XojPopplerPage.h"
#include "pdf/base/XojPdfDocumentInterface.h"

#include <StringUtils.h>

#include <boost/filesystem/path.hpp>
using boost::filesystem::path;

class _IntPopplerDocument;
class XojPopplerIter;

class XojPopplerDocument : public XojPdfDocumentInterface
{
public:
	XojPopplerDocument();
	XojPopplerDocument(const XojPopplerDocument& doc);
	virtual ~XojPopplerDocument();

public:
	void operator=(XojPopplerDocument& doc);
	bool operator==(XojPopplerDocument& doc);

	XojPdfBookmarkIterator* getContentsIter();

	XojPopplerPage* getPage(size_t page);

	bool isLoaded();

	size_t getPageCount();

	void load(char* data, int length);
	bool load(path filename, string password, GError** error);

	PDFDoc* getDoc();

	gsize getId();

	bool save(path filename, GError** error);

private:
	XOJ_TYPE_ATTRIB;

	_IntPopplerDocument* data;
};
