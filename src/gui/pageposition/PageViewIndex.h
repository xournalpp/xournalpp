/*
 * Xournal++
 *
 * Finds out on which page the most area of a rectangle is
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv3
 */

#pragma once

#include <XournalType.h>
#include <glib.h>

class PagePosition;
class PageView;

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

	GList* data;
};
