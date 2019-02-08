/*
 * Xournal++
 *
 * A page position (a vertical rect)
 *
 * @author Xournal++ Team, Justin Jones
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <XournalType.h>

class XojPageView;

/**
 * @brief A set of XojPageViews within an interval of y coordinates
 */
class PagePosition
{
public:
	PagePosition(XojPageView* pv);
	PagePosition();
	virtual ~PagePosition();

public:

	/**
	 * Returns whether or not the given x,y value is in
	 * this pageview
	 */
	bool containsPoint(int x, int y) const;


private:
	XOJ_TYPE_ATTRIB;

	// the minimal/maximal y and x  coordinates
	int y1;
	int y2;
	int x1;
	int x2;
	
	// the associated XojPageView
	XojPageView* pv;

	friend class PagePositionHandler;
};
