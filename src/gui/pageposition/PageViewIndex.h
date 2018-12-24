/*
 * Xournal++
 *
 * Finds out on which page the most area of a rectangle is
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <XournalType.h>

class PagePosition;
class XojPageView;
class PageViewIndexEntry;

class PageViewIndex
{
public:
	PageViewIndex(int x, int y, int width, int height);
	virtual ~PageViewIndex();

public:
	void add(PagePosition* pp, int y);

	XojPageView* getHighestIntersects();

private:
	void addView(XojPageView* v);

private:
	XOJ_TYPE_ATTRIB;

	int x;
	int y;
	int width;
	int height;

	std::vector<PageViewIndexEntry*> data;
};
