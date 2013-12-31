#include "DocumentHandler.h"
#include "DocumentListener.h"

DocumentHandler::DocumentHandler()
{
	XOJ_INIT_TYPE(DocumentHandler);

	this->listener = NULL;
}

DocumentHandler::~DocumentHandler()
{
	XOJ_CHECK_TYPE(DocumentHandler);

	// Do not delete the listeners!
	g_list_free(this->listener);
	this->listener = NULL;

	XOJ_RELEASE_TYPE(DocumentHandler);
}

void DocumentHandler::addListener(DocumentListener* l)
{
	XOJ_CHECK_TYPE(DocumentHandler);

	this->listener = g_list_append(this->listener, l);
}

void DocumentHandler::removeListener(DocumentListener* l)
{
	XOJ_CHECK_TYPE(DocumentHandler);

	this->listener = g_list_remove(this->listener, l);
}

void DocumentHandler::fireDocumentChanged(DocumentChangeType type)
{
	XOJ_CHECK_TYPE(DocumentHandler);

	for (GList* l = this->listener; l != NULL; l = l->next)
	{
		DocumentListener* dl = (DocumentListener*) l->data;
		dl->documentChanged(type);
	}
}

void DocumentHandler::firePageSizeChanged(int page)
{
	XOJ_CHECK_TYPE(DocumentHandler);

	for (GList* l = this->listener; l != NULL; l = l->next)
	{
		DocumentListener* dl = (DocumentListener*) l->data;
		dl->pageSizeChanged(page);
	}
}

void DocumentHandler::firePageChanged(int page)
{
	XOJ_CHECK_TYPE(DocumentHandler);

	for (GList* l = this->listener; l != NULL; l = l->next)
	{
		DocumentListener* dl = (DocumentListener*) l->data;
		dl->pageChanged(page);
	}
}

void DocumentHandler::firePageInserted(int page)
{
	XOJ_CHECK_TYPE(DocumentHandler);

	for (GList* l = this->listener; l != NULL; l = l->next)
	{
		DocumentListener* dl = (DocumentListener*) l->data;
		dl->pageInserted(page);
	}
}

void DocumentHandler::firePageDeleted(int page)
{
	for (GList* l = this->listener; l != NULL; l = l->next)
	{
		DocumentListener* dl = (DocumentListener*) l->data;
		dl->pageDeleted(page);
	}
}

void DocumentHandler::firePageSelected(int page)
{
	for (GList* l = this->listener; l != NULL; l = l->next)
	{
		DocumentListener* dl = (DocumentListener*) l->data;
		dl->pageSelected(page);
	}
}
