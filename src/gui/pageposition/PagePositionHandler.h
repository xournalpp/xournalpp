/*
 * Xournal++
 *
 * Knows the positions of pages in the view
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <XournalType.h>

#include <glib.h>

class PagePosition;
class PagePositionCache;
class XojPageView;

/**
 * @brief Look up XojPageView's from display coordinates
 * 
 * The PagePositionHandler maintains a set of PagePosition%s
 * sorted according to their respective intervals
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

	PagePosition* binarySearch(PagePosition** sortedArray, int first, int last, int y, int& index);

private:
	XOJ_TYPE_ATTRIB;

	int dataCount;
	int dataAllocSize;
	PagePosition** data;

	int maxY;
};
