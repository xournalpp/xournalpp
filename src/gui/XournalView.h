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

#include "control/ZoomControl.h"
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
class PageView;
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

	void requestPage(PageView* page);

	void layoutPages();

	void scrollTo(size_t pageNo, double y);

	size_t getCurrentPage();

	void updateXEvents();

	void clearSelection();

	void layerChanged(size_t page);

	void requestFocus();

	void forceUpdatePagenumbers();

	PageView* getViewFor(size_t pageNr);

	bool searchTextOnPage(string text, size_t p, int* occures, double* top);

	bool cut();
	bool copy();
	bool paste();

	void getPasteTarget(double& x, double& y);

	bool actionDelete();

	void endTextAllPages(PageView* except = NULL);

	void resetShapeRecognizer();

	int getDisplayWidth() const;
	int getDisplayHeight() const;

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
	void pageSelected(size_t page);
	void pageSizeChanged(size_t page);
	void pageChanged(size_t page);
	void pageInserted(size_t page);
	void pageDeleted(size_t page);
	void documentChanged(DocumentChangeType type);

public:
	bool onKeyPressEvent(GdkEventKey* event);
	bool onKeyReleaseEvent(GdkEventKey* event);

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

	PageView** viewPages;
	size_t viewPagesLen;

	Control* control;

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

	friend class Layout;
};
