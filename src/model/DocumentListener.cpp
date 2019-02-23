#include "DocumentListener.h"

#include "DocumentHandler.h"

DocumentListener::DocumentListener()
{
	XOJ_INIT_TYPE(DocumentListener);
}

DocumentListener::~DocumentListener()
{
	XOJ_CHECK_TYPE(DocumentListener);

	unregisterListener();

	XOJ_RELEASE_TYPE(DocumentListener);
}

void DocumentListener::registerListener(DocumentHandler* handler)
{
	XOJ_CHECK_TYPE(DocumentListener);

	this->handler = handler;
	handler->addListener(this);
}

void DocumentListener::unregisterListener()
{
	XOJ_CHECK_TYPE(DocumentListener);

	if (this->handler)
	{
		this->handler->removeListener(this);
	}
}

void DocumentListener::documentChanged(DocumentChangeType type)
{
	XOJ_CHECK_TYPE(DocumentListener);
}

void DocumentListener::pageSizeChanged(size_t page)
{
	XOJ_CHECK_TYPE(DocumentListener);
}

void DocumentListener::pageChanged(size_t page)
{
	XOJ_CHECK_TYPE(DocumentListener);
}

void DocumentListener::pageInserted(size_t page)
{
	XOJ_CHECK_TYPE(DocumentListener);
}

void DocumentListener::pageDeleted(size_t page)
{
	XOJ_CHECK_TYPE(DocumentListener);
}

void DocumentListener::pageSelected(size_t page)
{
	XOJ_CHECK_TYPE(DocumentListener);
}















