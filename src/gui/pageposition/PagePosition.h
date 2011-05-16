/*
 * Xournal++
 *
 * A page position (a vertical rect)
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __PAGEPOSITION_H__
#define __PAGEPOSITION_H__

#include <glib.h>
#include <XournalType.h>

class PageView;

class PagePosition {
public:
	PagePosition();
	PagePosition(PageView * pv);
	virtual ~PagePosition();

public:
	bool add(PageView * pv);
	bool containsY(int y);
	bool isYSmallerThan(int y);
	bool isYGraterThan(int y);

	PageView * getViewAt(int x, int y);

private:
	XOJ_TYPE_ATTRIB;

	int y1;
	int y2;

	GList * views;

	friend class PagePositionHandler;
};

#endif /* __PAGEPOSITION_H__ */
