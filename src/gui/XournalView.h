/*
 * Xournal++
 *
 * The widget wich displays the PDF and the drawings
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv3
 */

#ifndef __XOURNALVIEW_H__
#define __XOURNALVIEW_H__

#include <gtk/gtk.h>
#include <Arrayiterator.h>
#include "../control/ZoomControl.h"
#include "../model/DocumentListener.h"
#include "../model/PageRef.h"

class Control;
class PageView;
class Document;
class TextEditor;
class PdfCache;
class Rectangle;
class RepaintHandler;
class PagePositionHandler;
class Cursor;
class EditSelection;
class Layout;

class XournalView : public DocumentListener, public ZoomListener
{
public:
	XournalView(GtkWidget* parent, Control* control);
	virtual ~XournalView();

public:
	void zoomIn();
	void zoomOut();

	bool paint(GtkWidget* widget, GdkEventExpose* event);

	void requestPage(PageView* page);

	void layoutPages();

	void scrollTo(int pageNo, double y);

	int getCurrentPage();

	void updateXEvents();

	void clearSelection();

	void layerChanged(int page);

	void requestFocus();

	void forceUpdatePagenumbers();

	PageView* getViewFor(int pageNr);

	bool searchTextOnPage(string text, int p, int* occures, double* top);

	bool cut();
	bool copy();
	bool paste();

	void getPasteTarget(double& x, double& y);

	bool actionDelete();

	void endTextAllPages(PageView* except = NULL);

	void resetShapeRecognizer();

	bool isPageVisible(int page, int* visibleHeight);

	void ensureRectIsVisible(int x, int y, int width, int heigth);

	void setSelection(EditSelection* selection);
	EditSelection* getSelection();
	void deleteSelection(EditSelection* sel = NULL);
	void repaintSelection(bool evenWithoutSelection = false);

	TextEditor* getTextEditor();
	ArrayIterator<PageView*> pageViewIterator();
	Control* getControl();
	double getZoom();
	Document* getDocument();
	PagePositionHandler* getPagePositionHandler();
	PdfCache* getCache();
	RepaintHandler* getRepaintHandler();
	GtkWidget* getWidget();
	Cursor* getCursor();

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
	bool onKeyPressEvent(GdkEventKey* event);
	bool onKeyReleaseEvent(GdkEventKey* event);

private:

	void fireZoomChanged();

	void addLoadPageToQue(PageRef page, int priority);

	Rectangle* getVisibleRect(int page);

	static gboolean clearMemoryTimer(XournalView* widget);

private:
	XOJ_TYPE_ATTRIB;

	GtkWidget* widget;
	double margin;

	PageView** viewPages;
	int viewPagesLen;

	Control* control;

	int currentPage;
	int lastSelectedPage;

	PdfCache* cache;

	/**
	 * Handler for rerendering pages / repainting pages
	 */
	RepaintHandler* repaintHandler;

	/**
	 * The positions of all pages
	 */
	PagePositionHandler* pagePosition;

	/**
	 * Memory cleanup timeout
	 */
	int cleanupTimeout;

	friend class Layout;
};

#endif /* __XOURNALVIEW_H__ */
