#include "PagePositionHandler.h"
#include "PagePosition.h"
#include "PagePositionCache.h"

#include <gtk/gtk.h>

PagePositionHandler::PagePositionHandler() {
	this->dataCount = 0;
	this->dataAllocSize = 0;
	this->data = NULL;
	this->maxY = 0;
}

PagePositionHandler::~PagePositionHandler() {
	freeData();
}

void PagePositionHandler::freeData() {
	for (int i = 0; i < this->dataCount; i++) {
		delete this->data[i];
	}

	g_free(this->data);
	this->data = NULL;
	this->dataCount = 0;
	this->dataAllocSize = 0;
}

void PagePositionHandler::update(PageView ** viewPages, int viewPagesLen, int maxY) {
	freeData();

	this->maxY = maxY;

	PagePosition * lastPp = new PagePosition();
	addData(lastPp);

	for (int i = 0; i < viewPagesLen; i++) {
		PageView * pv = viewPages[i];
		if (!lastPp->add(pv)) {
			PagePosition * pp = new PagePosition(pv);
			lastPp->y2 = pp->y1 - 1;
			lastPp = pp;
			addData(pp);
		}
	}

	PagePosition * pp = new PagePosition();
	pp->y1 = lastPp->y2 + 1;
	pp->y2 = maxY;
	addData(pp);
}

PageView * PagePositionHandler::getViewAt(int x, int y, PagePositionCache * cache) {
	if (y < 0 || y > this->maxY) {
		return NULL;
	}

	if (cache && cache->ppId >= 0 && cache->ppId < this->dataCount) {
		if (this->data[cache->ppId]->containsY(y)) {
			return this->data[cache->ppId]->getViewAt(x, y);
		}
	}

	int index = -1;
	PagePosition * pp = binarySearch(this->data, 0, this->dataCount - 1, y, index);
	if (cache) {
		cache->ppId = index;
	}
	if (pp == NULL) {
		return NULL;
	}

	PageView * pv = pp->getViewAt(x, y);
	return pv;
}

PagePosition * PagePositionHandler::binarySearch(PagePosition ** sortedArray, int first, int last, int y, int & index) {
	while (first <= last) {
		int mid = (first + last) / 2; // compute mid point.
		if (sortedArray[mid]->isYSmallerThan(y)) {
			first = mid + 1; // repeat search in top half.
		} else if (sortedArray[mid]->isYGraterThan(y)) {
			last = mid - 1; // repeat search in bottom half.
		} else {
			index = mid;
			return sortedArray[mid]; // found it. return position
		}
	}
	return NULL; // nothing found
}

void PagePositionHandler::addData(PagePosition * p) {
	if (this->dataCount >= this->dataAllocSize) {
		this->allocDataSize(this->dataAllocSize + 100);
	}
	this->data[this->dataCount++] = p;
}

void PagePositionHandler::allocDataSize(int size) {
	this->dataAllocSize = size;
	this->data = (PagePosition **) g_realloc(this->data, this->dataAllocSize * sizeof(PagePosition *));
}
