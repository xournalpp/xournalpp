#include "LayoutData.h"

#include <gtk/gtk.h>

LayoutData::LayoutData()
{
	XOJ_INIT_TYPE(LayoutData);
	this->x = 0;
	this->y = 0;
	this->marginLeft = 0;
	this->marginTop = 0;
	this->pageIndex = 0;
}

LayoutData::~LayoutData()
{
	XOJ_RELEASE_TYPE(LayoutData);
}

int LayoutData::getX()
{
	XOJ_CHECK_TYPE(LayoutData);

	return this->x;
}

void LayoutData::setX(int x)
{
	XOJ_CHECK_TYPE(LayoutData);

	this->x = x;
}

int LayoutData::getY()
{
	XOJ_CHECK_TYPE(LayoutData);

	return this->y;
}

void LayoutData::setY(int y)
{
	XOJ_CHECK_TYPE(LayoutData);

	this->y = y;
}

int LayoutData::getPageIndex()
{
	XOJ_CHECK_TYPE(LayoutData);

	return this->pageIndex;
}

void LayoutData::setPageIndex(int pageIndex)
{
	XOJ_CHECK_TYPE(LayoutData);

	this->pageIndex = pageIndex;
}

int LayoutData::getLayoutAbsoluteX() const
{
	XOJ_CHECK_TYPE(LayoutData);

	return this->x + this->marginLeft;
}

int LayoutData::getLayoutAbsoluteY() const
{
	XOJ_CHECK_TYPE(LayoutData);

	return this->y + this->marginTop;
}

int LayoutData::getMarginLeft()
{
	XOJ_CHECK_TYPE(LayoutData);

	return this->marginLeft;
}

void LayoutData::setMarginLeft(int left)
{
	XOJ_CHECK_TYPE(LayoutData);

	this->marginLeft = left;
}

int LayoutData::getMarginTop()
{
	XOJ_CHECK_TYPE(LayoutData);

	return this->marginTop;
}

void LayoutData::setMarginTop(int top)
{
	XOJ_CHECK_TYPE(LayoutData);

	this->marginTop = top;
}
