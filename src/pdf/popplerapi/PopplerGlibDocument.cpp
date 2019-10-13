#include "PopplerGlibDocument.h"
#include "PopplerGlibPage.h"
#include "PopplerGlibPageBookmarkIterator.h"

#include <Util.h>
#include <memory>


PopplerGlibDocument::PopplerGlibDocument()
{
}

PopplerGlibDocument::PopplerGlibDocument(const PopplerGlibDocument& doc)
 : document(doc.document)
{
	if (document)
	{
		g_object_ref(document);
	}
}

PopplerGlibDocument::~PopplerGlibDocument()
{
	if (document)
	{
		g_object_unref(document);
		document = NULL;
	}
}

void PopplerGlibDocument::assign(XojPdfDocumentInterface* doc)
{
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
	return document == ((PopplerGlibDocument*)doc)->document;
}

bool PopplerGlibDocument::save(Path filename, GError** error)
{
	if (document == NULL)
	{
		return false;
	}

	string uri = filename.toUri(error);
	if (*error != NULL)
	{
		return false;
	}
	return poppler_document_save(document, uri.c_str(), error);
}

bool PopplerGlibDocument::load(Path filename, string password, GError** error)
{
	string uri = filename.toUri(error);
	if (*error != NULL)
	{
		return false;
	}

	if (document)
	{
		g_object_unref(document);
	}

	this->document = poppler_document_new_from_file(uri.c_str(), password.c_str(), error);
	return this->document != NULL;
}

bool PopplerGlibDocument::load(gpointer data, gsize length, string password, GError** error)
{
	if (document)
	{
		g_object_unref(document);
	}

	this->document = poppler_document_new_from_data(static_cast<char*>(data), static_cast<int>(length), password.c_str(), error);
	return this->document != NULL;
}

bool PopplerGlibDocument::isLoaded()
{
	return this->document != NULL;
}

XojPdfPageSPtr PopplerGlibDocument::getPage(size_t page)
{
	if (document == NULL)
	{
		return NULL;
	}

	PopplerPage* pg = poppler_document_get_page(document, page);
	XojPdfPageSPtr pageptr = std::make_shared<PopplerGlibPage>(pg);
	g_object_unref(pg);

	return pageptr;
}

size_t PopplerGlibDocument::getPageCount()
{
	if (document == NULL)
	{
		return 0;
	}

	return poppler_document_get_n_pages(document);
}

XojPdfBookmarkIterator* PopplerGlibDocument::getContentsIter()
{
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



