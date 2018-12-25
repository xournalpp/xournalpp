/*
 * Xournal++
 *
 * The widget wich displays the PDF and the drawings
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "control/zoom/ZoomListener.h"
#include "model/DocumentListener.h"
#include "model/PageRef.h"

#include <Arrayiterator.h>

#include <gtk/gtk.h>

class Control;
class Cursor;
class Document;
class EditSelection;
class Layout;
class PagePositionHandler;
class XojPageView;
class PdfCache;
class Rectangle;
class RepaintHandler;
class TextEditor;

class XournalView : public DocumentListener, public ZoomListener
{
public:
	XournalView(GtkWidget* parent, Control* control);
	virtual ~XournalView();

public:
	void zoomIn();
	void zoomOut();

	bool paint(GtkWidget* widget, GdkEventExpose* event);

	void requestPage(XojPageView* page);

	void layoutPages();

	void scrollTo(size_t pageNo, double y);

	size_t getCurrentPage();

	void clearSelection();

	void layerChanged(size_t page);

	void requestFocus();

	void forceUpdatePagenumbers();

	XojPageView* getViewFor(size_t pageNr);

	bool searchTextOnPage(string text, size_t p, int* occures, double* top);

	bool cut();
	bool copy();
	bool paste();

	void getPasteTarget(double& x, double& y);

	bool actionDelete();

	void endTextAllPages(XojPageView* except = NULL);

	void resetShapeRecognizer();

	int getDisplayWidth() const;
	int getDisplayHeight() const;

	bool isPageVisible(size_t page, int* visibleHeight);

	void ensureRectIsVisible(int x, int y, int width, int height);

	void setSelection(EditSelection* selection);
	EditSelection* getSelection();
	void deleteSelection(EditSelection* sel = NULL);
	void repaintSelection(bool evenWithoutSelection = false);

	void setEventCompression(gboolean enable);

	TextEditor* getTextEditor();
	ArrayIterator<XojPageView*> pageViewIterator();
	Control* getControl();
	double getZoom();
	int getDpiScaleFactor();
	Document* getDocument();
	PagePositionHandler* getPagePositionHandler();
	PdfCache* getCache();
	RepaintHandler* getRepaintHandler();
	GtkWidget* getWidget();
	Cursor* getCursor();

	Rectangle* getVisibleRect(int page);
	Rectangle* getVisibleRect(XojPageView* redrawable);

	GtkContainer* getParent();

	/**
	 * A pen action was detected now, therefore ignore touch events
	 * for a short time
	 */
	void penActionDetected();

	/**
	 * If the pen was active a short time before, ignore touch events
	 */
	bool shouldIgnoreTouchEvents();

public:
	// ZoomListener interface
	void zoomChanged();

public:
	// DocumentListener interface
	void pageSelected(size_t page);
	void pageSizeChanged(size_t page);
	void pageChanged(size_t page);
	void pageInserted(size_t page);
	void pageDeleted(size_t page);
	void documentChanged(DocumentChangeType type);

public:
	bool onKeyPressEvent(GdkEventKey* event);
	bool onKeyReleaseEvent(GdkEventKey* event);

	static void onRealized(GtkWidget* widget, XournalView* view);
private:

	void fireZoomChanged();

	void addLoadPageToQue(PageRef page, int priority);

	Rectangle* getVisibleRect(size_t page);

	static gboolean clearMemoryTimer(XournalView* widget);

	static void staticLayoutPages(GtkWidget *widget, GtkAllocation* allocation, void* data);

private:
	XOJ_TYPE_ATTRIB;

	GtkWidget* widget;
	double margin;

	XojPageView** viewPages;
	size_t viewPagesLen;

	Control* control;
	GtkContainer* parent;

	size_t currentPage;
	size_t lastSelectedPage;

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

	/**
	 * Last Pen action, to ignore touch events within a time frame
	 */
	gint64 lastPenAction;

	friend class Layout;
};
