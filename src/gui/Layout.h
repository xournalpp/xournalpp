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
	 * Adjusts the layout size to the given values
	 */
	void setSize(int widgetWidth, int widgetHeight);

	/**
	 * Increases the adjustments by the given amounts
	 */
	void scrollRelativ(int x, int y);

	/**
	 * Changes the adjustments by absolute amounts (for pinch-to-zoom)
	 */
	void scrollAbs(int x, int y);

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
	double getLayoutHeight();

	/**
	 * Returns the width of the entire Layout
	 */
	double getLayoutWidth();

	/**
	 * Returns the Rectangle which is currently visible
	 */
	Rectangle getVisibleRect();

	/**
	 * Performs a layout of the XojPageView's managed in this Layout
	 */
	void layoutPages();

	/**
	 * Updates the current XojPageView. The XojPageView is selected based on
	 * the percentage of the visible area of the XojPageView relative
	 * to its total area.
	 */
	void updateCurrentPage();

	
	
	/**
	 * Return the pageview containing co-ordinates.
	 */	
	XojPageView* getViewAt(int x, int y);

	
private:
	void checkScroll(GtkAdjustment* adjustment, double& lastScroll);
	void setLayoutSize(int width, int height);

private:
	XOJ_TYPE_ATTRIB;

	XournalView* view = NULL;	

	LayoutMapper mapper;

	ScrollHandling* scrollHandling = NULL;

	double lastScrollHorizontal = -1;
	double lastScrollVertical = -1;

	/**
	 * The last width of the widget
	 */
	int lastWidgetWidth = 0;

	/**
	 * The width and height of all our pages
	 */
	int layoutWidth = 0;
	int layoutHeight = 0;
	
	
	/**
	 *The following are useful for locating page at a pixel location
	 */
	
	int rows;
	int columns;
	
	std::vector<int> sizeCol;
	std::vector<int> sizeRow;
	
	/**
	 * cache the last GetViewAt() page#
	 */
	int lastGetViewAtPage;
	
};
