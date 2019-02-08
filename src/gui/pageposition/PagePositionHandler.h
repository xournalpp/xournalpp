/*
 * Xournal++
 *
 * Knows the positions of pages in the view
 *
 * @author Xournal++ Team, Justin Jones
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <XournalType.h>

class PagePosition;
class PagePositionCache;
class XojPageView;

/**
 * @brief Look up XojPageView's from display coordinates
 * 
 * The PagePositionHandler maintains a set of PagePosition's
 * not necessarily sorted.
 */
class PagePositionHandler
{
public:
	PagePositionHandler();
	virtual ~PagePositionHandler();

public:
	void update(XojPageView** viewPages, int viewPagesLen, int maxY);

	/**
	 * Returns the XojPageView with the given coordinates
	 */
	XojPageView* getViewAt(int x, int y, PagePositionCache* cache = NULL);
	XojPageView* getBestMatchingView(int x, int y, int width, int height);

private:
	void addData(PagePosition* p);
	void allocDataSize(int size);
	void freeData();

	// the PagePositions are no longer sorted for the binary search ( due to LayoutMapper ) but a brute force search of 300 pages is fine for when the cached result fails.
	PagePosition* linearSearch(int x, int y, int& index); 
	

private:
	XOJ_TYPE_ATTRIB;

	int dataCount;
	int dataAllocSize;
	PagePosition** data;

	int maxY;
};
