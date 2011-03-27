/*
 * Xournal++
 *
 * Finds out on which page the most area of a rectangle is
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __PAGEVIEWINDEX_H__
#define __PAGEVIEWINDEX_H__

#include "../../util/XournalType.h"
#include <glib.h>

class PagePosition;
class PageView;

class PageViewIndex {
public:
	PageViewIndex(int x, int y, int width, int height);
	virtual ~PageViewIndex();

public:
	void add(PagePosition * pp, int y);

	PageView * getHighestIntersects();

private:
	void addView(PageView * v);

private:
	XOJ_TYPE_ATTRIB;

	int x;
	int y;
	int width;
	int height;

	GList * data;
};

#endif /* __PAGEVIEWINDEX_H__ */
