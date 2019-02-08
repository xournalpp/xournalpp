#include "PagePositionHandler.h"

#include "PagePosition.h"
#include "PagePositionCache.h"



PagePositionHandler::PagePositionHandler()
{
	XOJ_INIT_TYPE(PagePositionHandler);

	dataCount = 0;
	dataAllocSize = 0;
	data = NULL;
	maxY = 0;
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

	for (int i = 0; i < dataCount; i++)
	{
		delete data[i];
	}

	g_free(data);
	data = NULL;
	dataCount = 0;
	dataAllocSize = 0;
}

void PagePositionHandler::update(XojPageView** viewPages, int viewPagesLen, int theMaxY)
{
	XOJ_CHECK_TYPE(PagePositionHandler);

	freeData();

	maxY = theMaxY;
	
	for (int i = 0; i < viewPagesLen; i++)
	{
		XojPageView* pv = viewPages[i];
		PagePosition* pp = new PagePosition(pv);
		addData(pp);
	}

}

XojPageView* PagePositionHandler::getBestMatchingView(int x, int y, int width, int height)
{
	XOJ_CHECK_TYPE(PagePositionHandler);

	// Does this simplification result in expected behaviour? 
	return getViewAt( x + width/2, y + height/2 );
}

XojPageView* PagePositionHandler::getViewAt(int x, int y, PagePositionCache* cache)
{
	XOJ_CHECK_TYPE(PagePositionHandler);

	if (y < 0 || y >maxY)
	{
		return NULL;
	}

	if (cache && cache->ppId >= 0 && cache->ppId < dataCount)
	{
		if (data[cache->ppId]->containsPoint(x, y))
		{
			return data[cache->ppId]->pv;
		}
	}

	int index = -1;
	PagePosition* pp = linearSearch(x,y, index);


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

PagePosition* PagePositionHandler::linearSearch( int x,int y, int& index)
{
	XOJ_CHECK_TYPE(PagePositionHandler);

		
			for ( int i = 0; i <dataCount; i++ )
			{
				if ( data[i]->containsPoint( x , y ) ){
					index = i;
					return data[i];
				}
			}
			
		
	return NULL; // nothing found
}



void PagePositionHandler::addData(PagePosition* p)
{
	XOJ_CHECK_TYPE(PagePositionHandler);

	if (dataCount >= dataAllocSize - 1)
	{
		allocDataSize(dataAllocSize + 100);
	}
	data[dataCount++] = p;
}

void PagePositionHandler::allocDataSize(int size)
{
	XOJ_CHECK_TYPE(PagePositionHandler);

	dataAllocSize = size;
	data = (PagePosition**) g_realloc(data, dataAllocSize * sizeof(PagePosition*));
}
