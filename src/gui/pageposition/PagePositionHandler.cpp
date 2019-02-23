#include "PagePositionHandler.h"

#include "PagePosition.h"
#include "PagePositionCache.h"


PagePositionHandler::PagePositionHandler()
{
	XOJ_INIT_TYPE(PagePositionHandler);
}

PagePositionHandler::~PagePositionHandler()
{
	XOJ_CHECK_TYPE(PagePositionHandler);

	this->freeData();

	XOJ_RELEASE_TYPE(PagePositionHandler);
}

void PagePositionHandler::freeData()
{
	XOJ_CHECK_TYPE(PagePositionHandler);

	for (int i = 0; i < this->dataCount; i++)
	{
		delete this->data[i];
	}

	g_free(this->data);
	this->data = NULL;
	this->dataCount = 0;
	this->dataAllocSize = 0;
}

void PagePositionHandler::update(XojPageView** viewPages, int viewPagesLen, int theMaxY)
{
	XOJ_CHECK_TYPE(PagePositionHandler);

	this->freeData();

	this->maxY = theMaxY;
	
	for (int i = 0; i < viewPagesLen; i++)
	{
		XojPageView* pv = viewPages[i];
		PagePosition* pp = new PagePosition(pv);
		this->addData(pp);
	}

}

XojPageView* PagePositionHandler::getBestMatchingView(int x, int y, int width, int height)
{
	XOJ_CHECK_TYPE(PagePositionHandler);

	// Does this simplification result in expected behaviour? 
	return this->getViewAt(x + width / 2, y + height / 2);
}

XojPageView* PagePositionHandler::getViewAt(int x, int y, PagePositionCache* cache)
{
	XOJ_CHECK_TYPE(PagePositionHandler);

	if (y < 0 || y > this->maxY)
	{
		return NULL;
	}

	if (cache && cache->ppId >= 0 && cache->ppId < this->dataCount)
	{
		if (this->data[cache->ppId]->containsPoint(x, y))
		{
			return this->data[cache->ppId]->pv;
		}
	}

	int index = -1;
	PagePosition* pp = this->linearSearch(x, y, index);

	if (cache)
	{
		cache->ppId = index;
	}
	if (pp == NULL)
	{
		return NULL;
	}

	return pp->pv;
}

PagePosition* PagePositionHandler::linearSearch(int x, int y, int& index)
{
	XOJ_CHECK_TYPE(PagePositionHandler);

	for (int i = 0; i < this->dataCount; i++)
	{
		if (this->data[i]->containsPoint(x, y))
		{
			index = i;
			return this->data[i];
		}
	}

	return NULL; // nothing found
}

void PagePositionHandler::addData(PagePosition* p)
{
	XOJ_CHECK_TYPE(PagePositionHandler);

	if (this->dataCount >= this->dataAllocSize - 1)
	{
		this->allocDataSize(this->dataAllocSize + 100);
	}
	this->data[this->dataCount++] = p;
}

void PagePositionHandler::allocDataSize(int size)
{
	XOJ_CHECK_TYPE(PagePositionHandler);

	this->dataAllocSize = size;
	this->data = (PagePosition**) g_realloc(this->data, this->dataAllocSize * sizeof(PagePosition*));
}
