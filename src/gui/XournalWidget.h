/*
 * Xournal++
 *
 * The widget wich displays the PDF and the drawings
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __XOURNALWIDGET_H__
#define __XOURNALWIDGET_H__

#include <gtk/gtk.h>

#include "../model/Document.h"
#include "PageView.h"
#include "../control/ZoomControl.h"
#include "../util/Arrayiterator.h"

const int XOURNAL_PADDING = 20;
const int XOURNAL_PADDING_TOP_LEFT = 10;

class Control;

class XournalWidget: public DocumentListener, public ZoomListener {
public:
	XournalWidget(GtkWidget * parent, Control * control);
	virtual ~XournalWidget();

public:
	void zoomIn();
	void zoomOut();

	bool paint(GtkWidget *widget, GdkEventExpose *event);

	GtkWidget * getWidget();

	void requestPage(PageView * page);

	void layoutPages();

	double getZoom();

	Document * getDocument();

	Control * getControl();

	void scrollTo(int pageNo, double y);

	int getCurrentPage();

	void updateXEvents();

	ArrayIterator<PageView *> pageViewIterator();

	void updateBackground();
	void layerChanged(int page);

	void resetFocus();

	void onScrolled();

	void forceUpdatePagenumbers();

	PageView * getViewFor(int pageNr);
	PageView * getViewAt(int x, int y);

	bool searchTextOnPage(const char * text, int p, int * occures, double * top);

	bool cut();
	bool copy();
	bool paste();

	void getPasteTarget(double & x, double & y);

	bool actionDelete();

	void endTextSelection();

public:
	//ZoomListener interface
	void zoomChanged(double lastZoom);

public:
	//DocumentListener interface
	void pageSelected(int page);
	void pageSizeChanged(int page);
	void pageChanged(int page);
	void pageInserted(int page);
	void pageDeleted(int page);
	void documentChanged(DocumentChangeType type);

private:
	static void onVscrollChanged(GtkAdjustment *adjustment, XournalWidget * xournal);

	void fireZoomChanged();

	void initScrollHandler(GtkWidget * parent);

	void addLoadPageToQue(XojPage * page, int priority);

	static void sizeAllocate(GtkWidget *widget, GtkRequisition *requisition, XournalWidget * xournal);
	static gboolean onButtonPressEventCallback(GtkWidget *widget, GdkEventButton *event, XournalWidget * xournal);

	static bool exposeEventCallback(GtkWidget *widget, GdkEventExpose *event, XournalWidget * xournal);
	void paintBorder(GtkWidget *widget, GdkEventExpose *event);

	static bool widgetRepaintCallback(GtkWidget * widget);
	static bool onKeyPressCallback(GtkWidget *widget, GdkEventKey *event, XournalWidget * xournal);
	static bool onKeyReleaseCallback(GtkWidget *widget, GdkEventKey *event, XournalWidget * xournal);
	bool onKeyPressEvent(GtkWidget *widget, GdkEventKey *event);
	bool onKeyReleaseEvent(GdkEventKey *event);
private:
	GtkWidget * widget;
	double margin;

	int lastWidgetSize;

	PageView ** viewPages;
	int viewPagesLen;

	Control * control;

	int currentPage;
	int lastSelectedPage;
};

#endif /* __XOURNALWIDGET_H__ */
