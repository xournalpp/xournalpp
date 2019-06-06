/*
 * Xournal++
 *
 * Handles the layout of the pages within a Xournal document
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <XournalType.h>
#include <Rectangle.h>

#include <gtk/gtk.h>

#include "gui/LayoutMapper.h"

class XojPageView;
class XournalView;
class ScrollHandling;


/**
 * @brief The Layout manager for the XournalWidget
 *
 * This class manages the layout of the XojPageView's contained
 * in the XournalWidget
 */
class Layout
{
public:
	Layout(XournalView* view, ScrollHandling* scrollHandling);
	virtual ~Layout();

public:
	/**
	 * Increases the adjustments by the given amounts
	 */
	void scrollRelative(double x, double y);

	/**
	 * Changes the adjustments by absolute amounts (for pinch-to-zoom)
	 */
	void scrollAbs(double x, double y);

	/**
	 * Changes the adjustments in such a way as to make sure that
	 * the given Rectangle is visible
	 *
	 * @remark If the given Rectangle won't fit into the scrolled window
	 *         then only its top left corner will be visible
	 */
	void ensureRectIsVisible(int x, int y, int width, int height);

	/**
	 * Returns the height of the entire Layout
	 */
	int getMinimalHeight();

	/**
	 * Returns the width of the entire Layout
	 */
	int getMinimalWidth();

	/**
	 * Returns the Rectangle which is currently visible
	 */
	Rectangle getVisibleRect();


	/**
	 * recalculate and resize window
	 * */
	void recalculate();

	/**
	 * Performs a layout of the XojPageView's managed in this Layout
	 * only call this on size allocation
	 */
	void layoutPages(int width, int height);

	/**
	 * Updates the current XojPageView. The XojPageView is selected based on
	 * the percentage of the visible area of the XojPageView relative
	 * to its total area.
	 */
	void updateVisibility();

	/**
	 * Return the pageview containing co-ordinates.
	 * 
	 */
	XojPageView* getViewAt(int x, int y);

	/**
	 * Return the page index found ( or -1 if not found) at layout grid row,col
	 * 
	 */
	int getIndexAtGridMap(int row, int col);

protected:
	static void horizontalScrollChanged(GtkAdjustment* adjustment, Layout* layout);
	static void verticalScrollChanged(GtkAdjustment* adjustment, Layout* layout);

private:
	void checkScroll(GtkAdjustment* adjustment, double& lastScroll);

	void setLayoutSize(int width, int height);

private:
	XOJ_TYPE_ATTRIB;

	LayoutMapper mapper;

	XournalView* view = nullptr;
	ScrollHandling* scrollHandling = nullptr;

	std::vector<int> widthCols;
	std::vector<int> heightRows;

	double lastScrollHorizontal = -1;
	double lastScrollVertical = -1;

	/**
	 * The last width and height of the widget
	 */
	int lastWidgetWidth = 0;
	int lastWidgetHeight = 0;

	/**
	 * The width and height of all our pages
	 */
	int minWidth = 0;
	int minHeight = 0;

	/**
	 * cache the last GetViewAt() row and column.
	 */
	int lastGetViewAtRow = 0;
	int lastGetViewAtCol = 0;

	bool valid = false;
};
