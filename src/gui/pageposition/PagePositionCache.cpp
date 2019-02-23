#include "PagePositionCache.h"

PagePositionCache::PagePositionCache()
{
	XOJ_INIT_TYPE(PagePositionCache);
}

PagePositionCache::~PagePositionCache()
{
	XOJ_CHECK_TYPE(PagePositionCache);

	this->ppId = -1;

	XOJ_RELEASE_TYPE(PagePositionCache);
}
