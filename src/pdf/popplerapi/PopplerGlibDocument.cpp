#include "PopplerGlibDocument.h"
#include "PopplerGlibPage.h"
#include "PopplerGlibPageBookmarkIterator.h"

#include <Util.h>
#include <memory>


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

string pathToUri(path filename, GError** error)
{
	char * uri = g_filename_to_uri(PATH_TO_CSTR(filename), NULL, error);

	if (uri == NULL)
	{
		return "";
	}

	string uriString = uri;
	g_free(uri);
	return uriString;
}

bool PopplerGlibDocument::save(path filename, GError** error)
{
	XOJ_CHECK_TYPE(PopplerGlibDocument);

	if (document == NULL)
	{
		return false;
	}

	string uri = pathToUri(filename, error);
	if (*error != NULL)
	{
		return false;
	}
	return poppler_document_save(document, uri.c_str(), error);
}

bool PopplerGlibDocument::load(path filename, string password, GError** error)
{
	XOJ_CHECK_TYPE(PopplerGlibDocument);

	string uri = pathToUri(filename, error);
	if (*error != NULL)
	{
		return false;
	}

	this->document = poppler_document_new_from_file(uri.c_str(), password.c_str(), error);
	return this->document != NULL;
}

bool PopplerGlibDocument::isLoaded()
{
	XOJ_CHECK_TYPE(PopplerGlibDocument);

	return this->document != NULL;
}

XojPdfPageSPtr PopplerGlibDocument::getPage(size_t page)
{
	XOJ_CHECK_TYPE(PopplerGlibDocument);

	if (document == NULL)
	{
		return NULL;
	}

	PopplerPage* pg = poppler_document_get_page(document, page);

	return std::make_shared<PopplerGlibPage>(pg);
}

size_t PopplerGlibDocument::getPageCount()
{
	XOJ_CHECK_TYPE(PopplerGlibDocument);

	if (document == NULL)
	{
		return 0;
	}

	return poppler_document_get_n_pages(document);
}

XojPdfBookmarkIterator* PopplerGlibDocument::getContentsIter()
{
	XOJ_CHECK_TYPE(PopplerGlibDocument);

	if (document == NULL)
	{
		return NULL;
	}

	PopplerIndexIter* iter = poppler_index_iter_new(document);

	if (iter == NULL)
	{
		return NULL;
	}

	return new PopplerGlibPageBookmarkIterator(iter, document);
}



