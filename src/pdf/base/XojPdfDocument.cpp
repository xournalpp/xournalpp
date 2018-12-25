#include "XojPdfDocument.h"

#include <config-features.h>

#include "pdf/popplerapi/PopplerGlibDocument.h"

XojPdfDocument::XojPdfDocument()
 : doc(new PopplerGlibDocument())
{
	XOJ_INIT_TYPE(XojPdfDocument);
}

XojPdfDocument::XojPdfDocument(const XojPdfDocument& doc)
 : doc(new PopplerGlibDocument())
{
	XOJ_INIT_TYPE(XojPdfDocument);
}

XojPdfDocument::~XojPdfDocument()
{
	XOJ_CHECK_TYPE(XojPdfDocument);

	delete doc;
	doc = NULL;

	XOJ_RELEASE_TYPE(XojPdfDocument);
}

void XojPdfDocument::operator=(XojPdfDocument& doc)
{
    XOJ_CHECK_TYPE(XojPdfDocument);

	this->doc->assign(doc.doc);
}

bool XojPdfDocument::operator==(XojPdfDocument& doc)
{
    XOJ_CHECK_TYPE(XojPdfDocument);

    return this->doc->equals(doc.doc);
}

void XojPdfDocument::assign(XojPdfDocumentInterface* doc)
{
	XOJ_CHECK_TYPE(XojPdfDocument);

	this->doc->assign(doc);
}

bool XojPdfDocument::equals(XojPdfDocumentInterface* doc)
{
	XOJ_CHECK_TYPE(XojPdfDocument);

    return this->doc->equals(doc);
}

bool XojPdfDocument::save(path filename, GError** error)
{
	XOJ_CHECK_TYPE(XojPdfDocument);

	return doc->save(filename, error);
}

bool XojPdfDocument::load(path filename, string password, GError** error)
{
	XOJ_CHECK_TYPE(XojPdfDocument);

	return doc->load(filename, password, error);
}

bool XojPdfDocument::isLoaded()
{
	XOJ_CHECK_TYPE(XojPdfDocument);

	return doc->isLoaded();
}

XojPdfPageSPtr XojPdfDocument::getPage(size_t page)
{
	XOJ_CHECK_TYPE(XojPdfDocument);

	return doc->getPage(page);
}

size_t XojPdfDocument::getPageCount()
{
	XOJ_CHECK_TYPE(XojPdfDocument);

	return doc->getPageCount();
}

XojPdfDocumentInterface* XojPdfDocument::getDocumentInterface()
{
	XOJ_CHECK_TYPE(XojPdfDocument);

	return doc;
}

XojPdfBookmarkIterator* XojPdfDocument::getContentsIter()
{
	XOJ_CHECK_TYPE(XojPdfDocument);

	return doc->getContentsIter();
}

