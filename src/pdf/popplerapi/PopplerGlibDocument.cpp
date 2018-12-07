#include "PopplerGlibDocument.h"
#include "PopplerGlibPage.h"


PopplerGlibDocument::PopplerGlibDocument()
 : document(NULL)
{
	XOJ_INIT_TYPE(PopplerGlibDocument);
}

PopplerGlibDocument::PopplerGlibDocument(const PopplerGlibDocument& doc)
 : document(doc.document)
{
	XOJ_INIT_TYPE(PopplerGlibDocument);

	if (document)
	{
		g_object_ref(document);
		document = NULL;
	}
}

PopplerGlibDocument::~PopplerGlibDocument()
{
	XOJ_CHECK_TYPE(PopplerGlibDocument);

	if (document)
	{
		g_object_unref(document);
	    document = NULL;
	}

	XOJ_RELEASE_TYPE(PopplerGlibDocument);
}

void PopplerGlibDocument::assign(XojPdfDocumentInterface* doc)
{
	XOJ_CHECK_TYPE(PopplerGlibDocument);

	if (document)
	{
		g_object_unref(document);
	}

	document = ((PopplerGlibDocument*)doc)->document;
	if (document)
	{
		g_object_ref(document);
	}
}

bool PopplerGlibDocument::equals(XojPdfDocumentInterface* doc)
{
	XOJ_CHECK_TYPE(PopplerGlibDocument);

	return document == ((PopplerGlibDocument*)doc)->document;
}

bool PopplerGlibDocument::save(path filename, GError** error)
{
	XOJ_CHECK_TYPE(PopplerGlibDocument);

	string uri = "file://";
	uri += filename.string();
	return poppler_document_save(document, uri.c_str(), error);
}

bool PopplerGlibDocument::load(path filename, string password, GError** error)
{
	XOJ_CHECK_TYPE(PopplerGlibDocument);

	string uri = "file://";
	uri += filename.string();
	this->document = poppler_document_new_from_file(uri.c_str(), password.c_str(), error);

	return this->document != NULL;
}

bool PopplerGlibDocument::isLoaded()
{
	XOJ_CHECK_TYPE(PopplerGlibDocument);

	return this->document != NULL;
}

XojPdfPage* PopplerGlibDocument::getPage(size_t page)
{
	XOJ_CHECK_TYPE(PopplerGlibDocument);

	PopplerPage* pg = poppler_document_get_page(document, page);

	// TODO The page will not be freed!

	return new PopplerGlibPage(pg);
}

size_t PopplerGlibDocument::getPageCount()
{
	XOJ_CHECK_TYPE(PopplerGlibDocument);

	return poppler_document_get_n_pages(document);
}

XojPdfBookmarkIterator* PopplerGlibDocument::getContentsIter()
{
	XOJ_CHECK_TYPE(PopplerGlibDocument);

	// TODO Implement
	return NULL;
}

