/*
 * Xournal++
 *
 * A page position (a vertical rect)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <XournalType.h>

#include <vector>

class XojPageView;

/**
 * @brief A set of XojPageViews within an interval of y coordinates
 */
class PagePosition
{
public:
	PagePosition();
	PagePosition(XojPageView* pv);
	virtual ~PagePosition();

public:
	/**
	 * Adds a XojPageView to this PagePosition provided that the
	 * y interval is not split up
	 * 
	 * @return whether or not the XojPageView was added
	 */
	bool add(XojPageView* pv);

	/**
	 * Returns whether or not the given y value is in
	 * the current interval
	 */
	bool containsY(int y) const;

	/**
	 * Returns whether the given y value is below the
	 * current interval
	 */
	bool isYSmallerThan(int y) const;

	/**
	 * Returns whether the given y value is above the
	 * current interval
	 */
	bool isYGraterThan(int y) const;

	/**
	 * Returns the XojPageView containing the given
	 * point display point
	 */
	XojPageView* getViewAt(int x, int y);

private:
	XOJ_TYPE_ATTRIB;

	// the minimal/maximal y coordinates
	int y1;
	int y2;

	// a list of XojPageView
	std::vector<XojPageView*> views;

	friend class PagePositionHandler;
};
