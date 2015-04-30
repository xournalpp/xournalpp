#include "DocumentListener.h"

#include "DocumentHandler.h"

DocumentListener::DocumentListener()
{
	XOJ_INIT_TYPE(DocumentListener);

	this->handler = NULL;
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
