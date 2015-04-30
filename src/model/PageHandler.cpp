#include "PageHandler.h"

#include "PageListener.h"

PageHandler::PageHandler()
{
	XOJ_INIT_TYPE(PageHandler);

	this->listener = NULL;
}

PageHandler::~PageHandler()
{
	XOJ_CHECK_TYPE(PageHandler);

	// Do not delete the listeners!
	g_list_free(this->listener);
	this->listener = NULL;

	XOJ_RELEASE_TYPE(PageHandler);
}

void PageHandler::addListener(PageListener* l)
{
	XOJ_CHECK_TYPE(PageHandler);

	this->listener = g_list_append(this->listener, l);
}

void PageHandler::removeListener(PageListener* l)
{
	XOJ_CHECK_TYPE(PageHandler);

	this->listener = g_list_remove(this->listener, l);
}

void PageHandler::fireRectChanged(Rectangle &rect)
{
	XOJ_CHECK_TYPE(PageHandler);

	for (GList* l = this->listener; l != NULL; l = l->next)
	{
		PageListener* pl = (PageListener*) l->data;
		pl->rectChanged(rect);
	}
}

void PageHandler::fireRangeChanged(Range &range)
{
	XOJ_CHECK_TYPE(PageHandler);

	for (GList* l = this->listener; l != NULL; l = l->next)
	{
		PageListener* pl = (PageListener*) l->data;
		pl->rangeChanged(range);
	}
}

void PageHandler::fireElementChanged(Element *elem)
{
	XOJ_CHECK_TYPE(PageHandler);

	for (GList* l = this->listener; l != NULL; l = l->next)
	{
		PageListener* pl = (PageListener*) l->data;
		pl->elementChanged(elem);
	}
}

void PageHandler::firePageChanged()
{
	XOJ_CHECK_TYPE(PageHandler);

	for (GList* l = this->listener; l != NULL; l = l->next)
	{
		PageListener* pl = (PageListener*) l->data;
		pl->pageChanged();
	}
}
