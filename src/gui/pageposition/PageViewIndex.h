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

#include <vector>

class PagePosition;
class PageView;
class PageViewIndexEntry;

class PageViewIndex
{
public:
	PageViewIndex(int x, int y, int width, int height);
	virtual ~PageViewIndex();

public:
	void add(PagePosition* pp, int y);

	PageView* getHighestIntersects();

private:
	void addView(PageView* v);

private:
	XOJ_TYPE_ATTRIB;

	int x;
	int y;
	int width;
	int height;

	std::vector<PageViewIndexEntry*> data;
};
