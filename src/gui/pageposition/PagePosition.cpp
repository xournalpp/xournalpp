#include "PagePosition.h"

#include "gui/PageView.h"

PagePosition::PagePosition(XojPageView* aPv)
{
	XOJ_INIT_TYPE(PagePosition);

	y1 = aPv->getY();
	y2 = y1 + aPv->getDisplayHeight();
	x1 = aPv->getX();
	x2 = x1 + aPv->getDisplayWidth();

	pv = aPv;
}

PagePosition::PagePosition()
{
	XOJ_INIT_TYPE(PagePosition);

	y1 = y2 = x1 =x2 = 0;
	pv = 0;	
	
}


PagePosition::~PagePosition()
{
	XOJ_CHECK_TYPE(PagePosition);

	XOJ_RELEASE_TYPE(PagePosition);
}



bool PagePosition::containsPoint(int x, int y) const
{
	XOJ_CHECK_TYPE(PagePosition);

	return (     y >= y1      &&       y <= y2      &&       x>= x1       &&       x <= x2    )  ;
}

