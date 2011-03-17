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

#ifndef __XOURNALVIEW_H__
#define __XOURNALVIEW_H__

#include <gtk/gtk.h>
#include "../util/Arrayiterator.h"
#include "../control/ZoomControl.h"
#include "../util/MemoryCheck.h"
#include "../model/DocumentListener.h"

const int XOURNAL_PADDING = 20;
const int XOURNAL_PADDING_TOP_LEFT = 10;

class Control;
class PageView;
class Document;
class TextEditor;
class PdfCache;
class XojPage;
class Rectangle;

class XournalView: public DocumentListener, public ZoomListener, public MemoryCheckObject {
public:
	XournalView(GtkWidget * parent, GtkRange * hrange, GtkRange * vrange, Control * control);
	virtual ~XournalView();

public:
	void zoomIn();
	void zoomOut();

	bool paint(GtkWidget * widget, GdkEventExpose * event);

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

	void layerChanged(int page);

	void requestFocus();

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

	TextEditor * getTextEditor();

	void resetShapeRecognizer();

	PdfCache * getCache();

	bool isPageVisible(int page);

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
	static void onVscrollChanged(GtkAdjustment * adjustment, XournalView * xournal);

	void fireZoomChanged();

	void initScrollHandler(GtkWidget * parent);

	void addLoadPageToQue(XojPage * page, int priority);

	Rectangle * getVisibleRect(int page);

	static bool widgetRepaintCallback(GtkWidget * widget);
	static bool onKeyPressCallback(GtkWidget * widget, GdkEventKey * event, XournalView * xournal);
	static bool onKeyReleaseCallback(GtkWidget * widget, GdkEventKey * event, XournalView * xournal);
	bool onKeyPressEvent(GtkWidget * widget, GdkEventKey * event);
	bool onKeyReleaseEvent(GdkEventKey * event);

	static gboolean clearMemoryTimer(XournalView * widget);
private:
	GtkWidget * widget;
	double margin;

	PageView ** viewPages;
	int viewPagesLen;

	Control * control;

	int currentPage;
	int lastSelectedPage;


	PdfCache * cache;

	/**
	 * Memory cleanup timeout
	 */
	int cleanupTimeout;
};

#endif /* __XOURNALVIEW_H__ */
