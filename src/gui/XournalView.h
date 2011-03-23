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
class RepaintHandler;
class PagePositionHandler;
class Cursor;
class EditSelection;

class XournalView: public DocumentListener, public ZoomListener, public MemoryCheckObject {
public:
	XournalView(GtkWidget * parent, GtkRange * hrange, GtkRange * vrange, Control * control);
	virtual ~XournalView();

public:
	void zoomIn();
	void zoomOut();

	bool paint(GtkWidget * widget, GdkEventExpose * event);

	void requestPage(PageView * page);

	void layoutPages();

	void scrollTo(int pageNo, double y);

	int getCurrentPage();

	void updateXEvents();

	void clearSelection();

	void layerChanged(int page);

	void requestFocus();

	void onScrolled();

	void forceUpdatePagenumbers();

	PageView * getViewFor(int pageNr);

	bool searchTextOnPage(const char * text, int p, int * occures, double * top);

	bool cut();
	bool copy();
	bool paste();

	void getPasteTarget(double & x, double & y);

	bool actionDelete();

	void endTextSelection();

	void resetShapeRecognizer();

	bool isPageVisible(int page);

	void ensureRectIsVisible(int x, int y, int width, int heigth);

	void setSelection(EditSelection * selection);
	EditSelection * getSelection();
	void deleteSelection();
	void repaintSelection();

	TextEditor * getTextEditor();
	ArrayIterator<PageView *> pageViewIterator();
	Control * getControl();
	double getZoom();
	Document * getDocument();
	PagePositionHandler * getPagePositionHandler();
	PdfCache * getCache();
	RepaintHandler * getRepaintHandler();
	GtkWidget * getWidget();
	Cursor * getCursor();

	int getMaxAreaX();
	int getMaxAreaY();

	/**
	 * This method is only internally used (by GtkXournal)
	 */
	void widgetDeleted();
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

public:
	bool onKeyPressEvent(GdkEventKey * event);
	bool onKeyReleaseEvent(GdkEventKey * event);

private:
	static void onVscrollChanged(GtkAdjustment * adjustment, XournalView * xournal);

	void fireZoomChanged();

	void addLoadPageToQue(XojPage * page, int priority);

	Rectangle * getVisibleRect(int page);

	static gboolean clearMemoryTimer(XournalView * widget);
private:
	XOJ_TYPE_ATTRIB;

	GtkWidget * widget;
	double margin;

	PageView ** viewPages;
	int viewPagesLen;

	Control * control;

	int currentPage;
	int lastSelectedPage;

	PdfCache * cache;

	/**
	 * Handler for rerendering pages / repainting pages
	 */
	RepaintHandler * repaintHandler;

	/**
	 * The positions of all pages
	 */
	PagePositionHandler * pagePosition;

	/**
	 * Memory cleanup timeout
	 */
	int cleanupTimeout;
};

#endif /* __XOURNALVIEW_H__ */
