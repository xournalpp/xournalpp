/*
 * Xournal++
 *
 * Handles the layout of the pages within a Xournal document
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __LAYOUT_H__
#define __LAYOUT_H__

#include <gtk/gtk.h>
#include <XournalType.h>
#include "widgets/Scrollbar.h" // because of extends ScrollbarListener

class PageView;
class XournalView;

class Layout : public ScrollbarListener {
public:
	Layout(XournalView * view);
	virtual ~Layout();

public:
	void setSize(int widgetWidth, int widgetHeight);
	void scrollRelativ(int x, int y);
	bool scrollEvent(GdkEventScroll * event);
	void ensureRectIsVisible(int x, int y, int width, int height);
	double getVisiblePageTop(int page);

	void layoutPages();

	GtkWidget * getScrollbarVertical();
	GtkWidget * getScrollbarHorizontal();

	void setLayoutSize(int width, int height);

	void updateRepaintWidget();

	virtual void scrolled(Scrollbar * scrollbar);

	void checkSelectedPage();

private:
	XOJ_TYPE_ATTRIB;

	XournalView * view;

	Scrollbar * scrollVertical;
	Scrollbar * scrollHorizontal;

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

#endif /* __LAYOUT_H__ */

