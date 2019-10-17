#include "DocumentHandler.h"

#include "DocumentListener.h"

DocumentHandler::DocumentHandler()
{
}

DocumentHandler::~DocumentHandler()
{
	// Do not delete the listeners!
}

void DocumentHandler::addListener(DocumentListener* l)
{
	this->listener.push_back(l);
}

void DocumentHandler::removeListener(DocumentListener* l)
{
	this->listener.remove(l);
}

void DocumentHandler::fireDocumentChanged(DocumentChangeType type)
{
	for (DocumentListener* dl : this->listener)
	{
		dl->documentChanged(type);
	}
}

void DocumentHandler::firePageSizeChanged(size_t page)
{
	for (DocumentListener* dl : this->listener)
	{
		dl->pageSizeChanged(page);
	}
}

void DocumentHandler::firePageChanged(size_t page)
{
	for (DocumentListener* dl : this->listener)
	{
		dl->pageChanged(page);
	}
}

void DocumentHandler::firePageInserted(size_t page)
{
	for (DocumentListener* dl : this->listener)
	{
		dl->pageInserted(page);
	}
}

void DocumentHandler::firePageDeleted(size_t page)
{
	for (DocumentListener* dl : this->listener)
	{
		dl->pageDeleted(page);
	}
}

void DocumentHandler::firePageSelected(size_t page)
{
	for (DocumentListener* dl : this->listener)
	{
		dl->pageSelected(page);
	}
}
