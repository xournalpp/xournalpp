#include "PagePositionHandler.h"
#include "PagePosition.h"
#include "PagePositionCache.h"
#include "PageViewIndex.h"

#include <gtk/gtk.h>

PagePositionHandler::PagePositionHandler()
{
	XOJ_INIT_TYPE(PagePositionHandler);

	this->dataCount = 0;
	this->dataAllocSize = 0;
	this->data = NULL;
	this->maxY = 0;
}

PagePositionHandler::~PagePositionHandler()
{
	XOJ_CHECK_TYPE(PagePositionHandler);

	freeData();

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

void PagePositionHandler::update(PageView** viewPages, int viewPagesLen,
								 int maxY)
{
	XOJ_CHECK_TYPE(PagePositionHandler);

	freeData();

	this->maxY = maxY;

	PagePosition* lastPp = new PagePosition();
	addData(lastPp);

	for (int i = 0; i < viewPagesLen; i++)
	{

		PageView* pv = viewPages[i];

		if (!lastPp->add(pv))
		{
			PagePosition* pp = new PagePosition(pv);
			lastPp->y2 = pp->y1 - 1;
			lastPp = pp;
			addData(pp);
		}
	}

	PagePosition* pp = new PagePosition();
	pp->y1 = lastPp->y2 + 1;
	pp->y2 = maxY;
	addData(pp);
}

PageView* PagePositionHandler::getBestMatchingView(int x, int y, int width,
												   int heigth)
{
	XOJ_CHECK_TYPE(PagePositionHandler);

	if (y + heigth < 0 || y > this->maxY)
	{
		return NULL;
	}

	int id = -1;
	PagePosition* pp1 = binarySearch(this->data, 0, this->dataCount - 1, y, id);
	id = -1;
	PagePosition* pp2 = binarySearch(this->data, 0, this->dataCount - 1, y + heigth,
									id);

	PageViewIndex index(x, y, width, heigth);
	if (pp1 != NULL)
	{
		index.add(pp1, y);
	}

	if (pp2 != NULL && pp1 != pp2)
	{
		index.add(pp2, y + heigth);
	}

	return index.getHighestIntersects();
}

PageView* PagePositionHandler::getViewAt(int x, int y,
										 PagePositionCache* cache)
{
	XOJ_CHECK_TYPE(PagePositionHandler);

	if (y < 0 || y > this->maxY)
	{
		return NULL;
	}

	if (cache && cache->ppId >= 0 && cache->ppId < this->dataCount)
	{
		if (this->data[cache->ppId]->containsY(y))
		{
			return this->data[cache->ppId]->getViewAt(x, y);
		}
	}

	int index = -1;
	PagePosition* pp = binarySearch(this->data, 0, this->dataCount - 1, y, index);


	if (cache)
	{
		cache->ppId = index;
	}
	if (pp == NULL)
	{
		return NULL;
	}

	PageView* pv = pp->getViewAt(x, y);
	return pv;
}

PagePosition* PagePositionHandler::binarySearch(PagePosition** sortedArray,
												int first, int last, int y, int& index)
{
	XOJ_CHECK_TYPE(PagePositionHandler);

	while (first <= last)
	{
		int mid = (first + last) / 2; // compute mid point.
		if (sortedArray[mid]->isYSmallerThan(y))
		{
			first = mid + 1; // repeat search in top half.
		}
		else if (sortedArray[mid]->isYGraterThan(y))
		{
			last = mid - 1; // repeat search in bottom half.
		}
		else
		{
			index = mid;
			return sortedArray[mid]; // found it. return position
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
	this->data = (PagePosition**) g_realloc(this->data,
											this->dataAllocSize * sizeof (PagePosition*));
}
