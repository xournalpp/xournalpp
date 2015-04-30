#include "DocumentHandler.h"

#include "DocumentListener.h"

#define FOR_ALL for (DocumentListener* dl : this->listener)

DocumentHandler::DocumentHandler()
{
	XOJ_INIT_TYPE(DocumentHandler);
}

DocumentHandler::~DocumentHandler()
{
	XOJ_CHECK_TYPE(DocumentHandler);

	// Do not delete the listeners!

	XOJ_RELEASE_TYPE(DocumentHandler);
}

void DocumentHandler::addListener(DocumentListener* l)
{
	XOJ_CHECK_TYPE(DocumentHandler);

	this->listener.push_back(l);
}

void DocumentHandler::removeListener(DocumentListener* l)
{
	XOJ_CHECK_TYPE(DocumentHandler);

	this->listener.remove(l);
}

void DocumentHandler::fireDocumentChanged(DocumentChangeType type)
{
	XOJ_CHECK_TYPE(DocumentHandler);

	FOR_ALL dl->documentChanged(type);
}

void DocumentHandler::firePageSizeChanged(int page)
{
	XOJ_CHECK_TYPE(DocumentHandler);

	FOR_ALL dl->pageSizeChanged(page);
}

void DocumentHandler::firePageChanged(int page)
{
	XOJ_CHECK_TYPE(DocumentHandler);

	FOR_ALL dl->pageChanged(page);
}

void DocumentHandler::firePageInserted(int page)
{
	XOJ_CHECK_TYPE(DocumentHandler);

	FOR_ALL dl->pageInserted(page);
}

void DocumentHandler::firePageDeleted(int page)
{
	FOR_ALL dl->pageDeleted(page);
}

void DocumentHandler::firePageSelected(int page)
{
	FOR_ALL dl->pageSelected(page);
}
