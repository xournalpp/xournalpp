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

#include "widgets/Scrollbar.h" // because of extends ScrollbarListener
#include <XournalType.h>

#include <gtk/gtk.h>

class PageView;
class XournalView;

/**
 * @brief The Layout manager for the XournalWidget
 *
 * This class manages the layout of the PageView%s contained
 * in the XournalWidget
 */
class Layout : public ScrollbarListener
{
public:
	Layout(XournalView* view);
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
	 * Handle a scroll event
	 */
	bool scrollEvent(GdkEventScroll* event);

	/**
	 * Changes the adjustments in such a way as to make sure that
	 * the given Rectangle is visible
	 *
	 * @remark If the given Rectangle won't fit into the scrolled window
	 *         then only its top left corner will be visible
	 */
	void ensureRectIsVisible(int x, int y, int width, int height);

	double getVisiblePageTop(size_t page);
	double getDisplayHeight();

	void layoutPages();

	GtkWidget* getScrollbarVertical();
	GtkWidget* getScrollbarHorizontal();

	void setLayoutSize(int width, int height);

	void updateRepaintWidget();

	virtual void scrolled(Scrollbar* scrollbar);

	void checkSelectedPage();

private:
	XOJ_TYPE_ATTRIB;

	XournalView* view;

	Scrollbar* scrollVertical;
	Scrollbar* scrollHorizontal;

	/**
	 * Outer border of the complete layout
	 */
	int marginTop;
	int marginLeft;
	int marginRight;
	int marginBottom;

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
