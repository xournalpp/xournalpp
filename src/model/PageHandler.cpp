#include "PageHandler.h"

#include "PageListener.h"

PageHandler::PageHandler()
{
	XOJ_INIT_TYPE(PageHandler);
}

PageHandler::~PageHandler()
{
	XOJ_CHECK_TYPE(PageHandler);

	XOJ_RELEASE_TYPE(PageHandler);
}

void PageHandler::addListener(PageListener* l)
{
	XOJ_CHECK_TYPE(PageHandler);

	this->listener.push_back(l);
}

void PageHandler::removeListener(PageListener* l)
{
	XOJ_CHECK_TYPE(PageHandler);

	this->listener.remove(l);
}

void PageHandler::fireRectChanged(Rectangle &rect)
{
	XOJ_CHECK_TYPE(PageHandler);

	for (PageListener* pl : this->listener)
	{
		pl->rectChanged(rect);
	}
}

void PageHandler::fireRangeChanged(Range &range)
{
	XOJ_CHECK_TYPE(PageHandler);

	for (PageListener* pl : this->listener)
	{
		pl->rangeChanged(range);
	}
}

void PageHandler::fireElementChanged(Element *elem)
{
	XOJ_CHECK_TYPE(PageHandler);

	for (PageListener* pl : this->listener)
	{
		pl->elementChanged(elem);
	}
}

void PageHandler::firePageChanged()
{
	XOJ_CHECK_TYPE(PageHandler);

	for (PageListener* pl : this->listener)
	{
		pl->pageChanged();
	}
}
