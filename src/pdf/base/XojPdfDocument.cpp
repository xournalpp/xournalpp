#include "XojPdfDocument.h"

#include "pdf/popplerdirect/poppler/XojPopplerIter.h"

XojPdfDocument::XojPdfDocument()
{
    XOJ_INIT_TYPE(XojPdfDocument);
}

XojPdfDocument::XojPdfDocument(const XojPdfDocument& doc)
{
    XOJ_INIT_TYPE(XojPdfDocument);
}

XojPdfDocument::~XojPdfDocument()
{
    XOJ_RELEASE_TYPE(XojPdfDocument);
}

void XojPdfDocument::operator=(XojPdfDocument& doc)
{
    XOJ_CHECK_TYPE(XojPdfDocument);

	this->doc = doc.doc;
}

bool XojPdfDocument::operator==(XojPdfDocument& doc)
{
    XOJ_CHECK_TYPE(XojPdfDocument);

    return this->doc == doc.doc;
}

bool XojPdfDocument::save(path filename, GError** error)
{
    XOJ_CHECK_TYPE(XojPdfDocument);

    return doc.save(filename, error);
}

bool XojPdfDocument::load(path filename, string password, GError** error)
{
    XOJ_CHECK_TYPE(XojPdfDocument);

	return doc.load(filename, password, error);
}

bool XojPdfDocument::isLoaded()
{
    XOJ_CHECK_TYPE(XojPdfDocument);

    return doc.isLoaded();
}

XojPdfPage* XojPdfDocument::getPage(size_t page)
{
    XOJ_CHECK_TYPE(XojPdfDocument);

    return doc.getPage(page);
}

size_t XojPdfDocument::getPageCount()
{
    XOJ_CHECK_TYPE(XojPdfDocument);

    return doc.getPageCount();
}

XojPopplerDocument& XojPdfDocument::getPopplerDocument()
{
    XOJ_CHECK_TYPE(XojPdfDocument);

    return doc;
}

XojPdfBookmarkIterator* XojPdfDocument::getContentsIter()
{
    XOJ_CHECK_TYPE(XojPdfDocument);

    return doc.getContentsIter();
}

