#include "PageListener.h"
#include "PageHandler.h"

PageListener::PageListener()
{
	XOJ_INIT_TYPE(PageListener);

	this->handler = NULL;
}

PageListener::~PageListener()
{
	XOJ_CHECK_TYPE(PageListener);

	unregisterListener();

	XOJ_RELEASE_TYPE(PageListener);
}

void PageListener::registerListener(PageHandler* handler)
{
	XOJ_CHECK_TYPE(PageListener);

	this->handler = handler;
	handler->addListener(this);
}

void PageListener::unregisterListener()
{
	XOJ_CHECK_TYPE(PageListener);

	if (this->handler)
	{
		this->handler->removeListener(this);
	}
}

