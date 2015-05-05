#include "DocumentHandler.h"

#include "DocumentListener.h"

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

	for (DocumentListener* dl : this->listener)
	{
		dl->documentChanged(type);
	}
}

void DocumentHandler::firePageSizeChanged(int page)
{
	XOJ_CHECK_TYPE(DocumentHandler);

	for (DocumentListener* dl : this->listener)
	{
		dl->pageSizeChanged(page);
	}
}

void DocumentHandler::firePageChanged(int page)
{
	XOJ_CHECK_TYPE(DocumentHandler);

	for (DocumentListener* dl : this->listener)
	{
		dl->pageChanged(page);
	}
}

void DocumentHandler::firePageInserted(int page)
{
	XOJ_CHECK_TYPE(DocumentHandler);

	for (DocumentListener* dl : this->listener)
	{
		dl->pageInserted(page);
	}
}

void DocumentHandler::firePageDeleted(int page)
{
	for (DocumentListener* dl : this->listener)
	{
		dl->pageDeleted(page);
	}
}

void DocumentHandler::firePageSelected(int page)
{
	for (DocumentListener* dl : this->listener)
	{
		dl->pageSelected(page);
	}
}
