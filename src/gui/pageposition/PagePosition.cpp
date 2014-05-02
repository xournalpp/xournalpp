#include "PagePosition.h"

#include "../PageView.h"

PagePosition::PagePosition(PageView* pv)
{
	XOJ_INIT_TYPE(PagePosition);

	this->y1 = pv->getY();
	this->y2 = this->y1 + pv->getDisplayHeight();

	this->views = g_list_append(NULL, pv);
}

PagePosition::PagePosition()
{
	XOJ_INIT_TYPE(PagePosition);

	this->y1 = 0;
	this->y2 = 0;

	this->views = NULL;
}

PagePosition::~PagePosition()
{
	XOJ_CHECK_TYPE(PagePosition);

	g_list_free(this->views);
	this->views = NULL;

	XOJ_RELEASE_TYPE(PagePosition);
}

bool PagePosition::add(PageView* pv)
{
	XOJ_CHECK_TYPE(PagePosition);

	int y1 = pv->getY();
	int y2 = y1 + pv->getDisplayHeight();

	if (containsY(y1) || containsY(y2) || pv->containsY(this->y1) ||
	    pv->containsY(this->y2))
	{
		this->views = g_list_append(this->views, pv);

		this->y1 = MIN(this->y1, y1);
		this->y2 = MAX(this->y2, y2);

		return true;
	}

	return false;
}

PageView* PagePosition::getViewAt(int x, int y)
{
	XOJ_CHECK_TYPE(PagePosition);

	for (GList* l = this->views; l != NULL; l = l->next)
	{
		PageView* v = (PageView*) l->data;
		if (v->containsPoint(x, y))
		{
			return v;
		}
	}
	return NULL;
}

bool PagePosition::containsY(int y) const
{
	XOJ_CHECK_TYPE(PagePosition);

	return (y >= this->y1 && y <= this->y2);
}

bool PagePosition::isYSmallerThan(int y) const
{
	XOJ_CHECK_TYPE(PagePosition);

	return y > this->y2;
}

bool PagePosition::isYGraterThan(int y) const
{
	XOJ_CHECK_TYPE(PagePosition);

	return y < this->y1;
}

