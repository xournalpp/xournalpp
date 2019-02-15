#include "PagePosition.h"

#include "gui/PageView.h"

PagePosition::PagePosition(XojPageView* aPv)
{
	XOJ_INIT_TYPE(PagePosition);

	this->y1 = aPv->getY();
	this->y2 = this->y1 + aPv->getDisplayHeight();
	this->x1 = aPv->getX();
	this->x2 = this->x1 + aPv->getDisplayWidth();

	this->pv = aPv;
}

PagePosition::PagePosition()
{
	XOJ_INIT_TYPE(PagePosition);

	this->y1 = 0;
	this->y2 = 0;
	this->x1 = 0;
	this->x2 = 0;
	this->pv = 0;
}

PagePosition::~PagePosition()
{
	XOJ_CHECK_TYPE(PagePosition);

	XOJ_RELEASE_TYPE(PagePosition);
}

bool PagePosition::containsPoint(int x, int y) const
{
	XOJ_CHECK_TYPE(PagePosition);

	return y >= this->y1 && y <= this->y2 && x >= this->x1 && x <= this->x2;
}

