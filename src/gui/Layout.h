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

class PageView;
class XournalView;

/**
 * @brief The Layout manager for the XournalWidget
 *
 * This class manages the layout of the PageView%s contained
 * in the XournalWidget
 */
class Layout
{
public:
	Layout(XournalView* view,
	       GtkAdjustment* adjHorizontal,
	       GtkAdjustment* adjVertical);
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
	double getLayoutHeight()
	{
		return layoutHeight;
	}

	/**
	 * Returns the width of the entire Layout
	 */
	double getLayoutWidth()
	{
		return layoutWidth;
	}

	/**
	 * Returns the Rectangle which is currently visible
	 */
	Rectangle getVisibleRect();

	/**
	 * Performs a layout of the PageView%s managed in this Layout
	 */
	void layoutPages();

	/**
	 * Updates the current PageView. The PageView is selected based on
	 * the percentage of the visible area of the PageView relative
	 * to its total area.
	 */
	void updateCurrentPage();

public:
	/**
	 * Callback for a changed value of one of the adjustments of
	 * the scrolled window. It updates the current PageView
	 * via Layout::updateCurrentPage()
	 */
	static void adjustmentValueChanged(GtkAdjustment* adjustment,
	                                   Layout* layout);

private:
	void setLayoutSize(int width, int height);

	XOJ_TYPE_ATTRIB;

	XournalView* view;

	GtkAdjustment* adjHorizontal;
	GtkAdjustment* adjVertical;

	/**
	 * The last width of the widget
	 */
	int lastWidgetWidth;

	/**
	 * The width and height of all our pages
	 */
	int layoutWidth;
	int layoutHeight;
};
