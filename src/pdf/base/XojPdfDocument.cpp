#include "XojPdfDocument.h"

#include <config-features.h>

#include "pdf/popplerapi/PopplerGlibDocument.h"

XojPdfDocument::XojPdfDocument()
 : doc(new PopplerGlibDocument())
{
}

XojPdfDocument::XojPdfDocument(const XojPdfDocument& doc)
 : doc(new PopplerGlibDocument())
{
}

XojPdfDocument::~XojPdfDocument()
{
	delete doc;
	doc = nullptr;
}

XojPdfDocument& XojPdfDocument::operator=(const XojPdfDocument& doc)
{
	this->doc->assign(doc.doc);
	return *this;
}

bool XojPdfDocument::operator==(XojPdfDocument& doc)
{
	return this->doc->equals(doc.doc);
}

void XojPdfDocument::assign(XojPdfDocumentInterface* doc)
{
	this->doc->assign(doc);
}

bool XojPdfDocument::equals(XojPdfDocumentInterface* doc)
{
	return this->doc->equals(doc);
}

bool XojPdfDocument::save(Path filename, GError** error)
{
	return doc->save(filename, error);
}

bool XojPdfDocument::load(Path filename, string password, GError** error)
{
	return doc->load(filename, password, error);
}

bool XojPdfDocument::load(gpointer data, gsize length, string password, GError** error)
{
	return doc->load(data, length, password, error);
}

bool XojPdfDocument::isLoaded()
{
	return doc->isLoaded();
}

XojPdfPageSPtr XojPdfDocument::getPage(size_t page)
{
	return doc->getPage(page);
}

size_t XojPdfDocument::getPageCount()
{
	return doc->getPageCount();
}

XojPdfDocumentInterface* XojPdfDocument::getDocumentInterface()
{
	return doc;
}

XojPdfBookmarkIterator* XojPdfDocument::getContentsIter()
{
	return doc->getContentsIter();
}

