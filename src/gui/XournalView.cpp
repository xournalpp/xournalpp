#include "XournalView.h"
#include "../control/Control.h"
#include <math.h>
#include "Shadow.h"
#include "../util/Util.h"

#include "../model/Document.h"
#include "../model/Stroke.h"
#include "PageView.h"
#include "../control/PdfCache.h"
#include "../control/settings/MetadataManager.h"
#include "../util/Rectangle.h"
#include "widgets/XournalWidget.h"
#include "pageposition/PagePositionHandler.h"
#include "Cursor.h"
#include "../undo/DeleteUndoAction.h"

#include "RepaintHandler.h"

#include <gdk/gdkkeysyms.h>

XournalView::XournalView(GtkWidget * parent, GtkRange * hrange, GtkRange * vrange, Control * control) {
	this->control = control;
	this->cache = new PdfCache(control->getSettings()->getPdfPageCacheSize());
	registerListener(control);

	this->widget = gtk_xournal_new(this, hrange, vrange);

	gtk_table_attach_defaults(GTK_TABLE(parent), this->widget, 1, 2, 0, 1);
	gtk_widget_show(this->widget);

	this->repaintHandler = new RepaintHandler(this);
	this->pagePosition = new PagePositionHandler();

	this->viewPages = NULL;
	this->viewPagesLen = 0;
	this->margin = 75;
	this->currentPage = 0;
	this->lastSelectedPage = -1;

	GtkAdjustment * adj = vrange->adjustment;
	g_signal_connect(adj, "value-changed", G_CALLBACK(onVscrollChanged), this);

	control->getZoomControl()->addZoomListener(this);

	gtk_widget_set_can_default(this->widget, true);
	gtk_widget_grab_default(this->widget);

	gtk_widget_grab_focus(this->widget);

	this->cleanupTimeout = g_timeout_add_seconds(5, (GSourceFunc) clearMemoryTimer, this);
}

XournalView::~XournalView() {
	g_source_remove(this->cleanupTimeout);
	delete this->cache;
	this->cache = NULL;
	delete this->repaintHandler;
	this->repaintHandler = NULL;

	delete this->pagePosition;
	this->pagePosition = NULL;
}

gint pageViewCmpSize(PageView * a, PageView * b) {
	return a->getLastVisibelTime() - b->getLastVisibelTime();
}

gboolean XournalView::clearMemoryTimer(XournalView * widget) {
	GList * list = NULL;

	for (int i = 0; i < widget->viewPagesLen; i++) {
		PageView * v = widget->viewPages[i];
		if (v->getLastVisibelTime() > 0) {
			list = g_list_insert_sorted(list, v, (GCompareFunc) pageViewCmpSize);
		}
	}

	int pixel = 2884560;
	int firstPages = 4;

	int i = 0;

	for (GList * l = list; l != NULL; l = l->next) {
		if (firstPages) {
			firstPages--;
		} else {
			PageView * v = (PageView *) l->data;

			if (pixel <= 0) {
				v->deleteViewBuffer();
			} else {
				pixel -= v->getBufferPixels();
			}
		}
		i++;
	}

	g_list_free(list);

	// call again
	return true;
}

void XournalView::onVscrollChanged(GtkAdjustment * adjustment, XournalView * xournal) {
	xournal->onScrolled();
}

int XournalView::getCurrentPage() {
	return currentPage;
}

const int scrollKeySize = 10;

bool XournalView::onKeyPressEvent(GdkEventKey * event) {
	int p = getCurrentPage();
	if (p >= 0 && p < this->viewPagesLen) {
		PageView * v = this->viewPages[p];
		if (v->onKeyPressEvent(event)) {
			return true;
		}
	}

	// Esc leaves fullscreen mode
	if (event->keyval == GDK_Escape || event->keyval == GDK_F11) {
		if (control->isFullscreen()) {
			control->enableFullscreen(false);
			return true;
		}
	}

	// F5 starts presentation modus
	if (event->keyval == GDK_F5) {
		if (!control->isFullscreen()) {
			control->enableFullscreen(true, true);
			return true;
		}
	}

	guint state = event->state & gtk_accelerator_get_default_mod_mask();

	if (state & GDK_SHIFT_MASK) {
		if (event->keyval == GDK_Page_Down) {
			control->getScrollHandler()->goToNextPage();
			return true;
		}
		if (event->keyval == GDK_Page_Up) {
			control->getScrollHandler()->goToPreviousPage();
			return true;
		}
	} else {
		GtkAllocation alloc = { 0 };
		gtk_widget_get_allocation(gtk_widget_get_parent(this->widget), &alloc);
		int windowHeight = alloc.height - scrollKeySize;

		if (event->keyval == GDK_Page_Down) {
			gtk_xournal_scroll_relative(this->widget, 0, windowHeight);
			return true;
		}
		if (event->keyval == GDK_Page_Up) {
			gtk_xournal_scroll_relative(this->widget, 0, -windowHeight);
			return true;
		}
	}

	if (event->keyval == GDK_Up) {
		gtk_xournal_scroll_relative(this->widget, 0, -scrollKeySize);
		return true;
	}

	if (event->keyval == GDK_Down) {
		gtk_xournal_scroll_relative(this->widget, 0, scrollKeySize);
		return true;
	}

	if (event->keyval == GDK_Left) {
		gtk_xournal_scroll_relative(this->widget, -scrollKeySize, 0);
		return true;
	}

	if (event->keyval == GDK_Right) {
		gtk_xournal_scroll_relative(this->widget, scrollKeySize, 0);
		return true;
	}

	return false;
}

RepaintHandler * XournalView::getRepaintHandler() {
	return this->repaintHandler;
}

bool XournalView::onKeyReleaseEvent(GdkEventKey *event) {
	int p = getCurrentPage();
	if (p >= 0 && p < this->viewPagesLen) {
		PageView * v = this->viewPages[p];
		if (v->onKeyReleaseEvent(event)) {
			return true;
		}
	}

	return false;
}

// send the focus back to the appropriate widget
void XournalView::requestFocus() {
	gtk_widget_grab_focus(this->widget);
}

bool XournalView::searchTextOnPage(const char * text, int p, int * occures, double * top) {
	if (p < 0 || p >= this->viewPagesLen) {
		return false;
	}
	PageView * v = this->viewPages[p];

	return v->searchTextOnPage(text, occures, top);
}

void XournalView::forceUpdatePagenumbers() {
	int p = this->currentPage;
	this->currentPage = -1;

	control->firePageSelected(p);
}

PageView * XournalView::getViewFor(int pageNr) {
	if (pageNr < 0 || pageNr >= this->viewPagesLen) {
		return NULL;
	}
	return this->viewPages[pageNr];
}

void XournalView::pageSelected(int page) {
	if (this->currentPage == page && this->lastSelectedPage == page) {
		return;
	}

	Document * doc = control->getDocument();
	doc->lock();
	String file = doc->getEvMetadataFilename();
	doc->unlock();

	control->getMetadataManager()->setInt(file, "page", page);

	if (this->lastSelectedPage >= 0 && this->lastSelectedPage < this->viewPagesLen) {
		this->viewPages[this->lastSelectedPage]->setSelected(false);
	}

	this->currentPage = page;

	int pdfPage = -1;

	if (page >= 0 && page < viewPagesLen) {
		PageView * vp = viewPages[page];
		vp->setSelected(true);
		lastSelectedPage = page;
		pdfPage = vp->getPage()->getPdfPageNr();
	}

	control->updatePageNumbers(currentPage, pdfPage);

	control->updateBackgroundSizeButton();
}

Control * XournalView::getControl() {
	return control;
}

void XournalView::scrollTo(int pageNo, double y) {
	if (this->currentPage == pageNo) {
		return;
	}
	if (pageNo < 0 || pageNo >= this->viewPagesLen) {
		return;
	}
	// TODO LOW PRIO: handle horizontal scrolling (dual page view)

	GtkAdjustment * h = gtk_xournal_get_vadj(this->widget);

	PageView * p = viewPages[pageNo];

	int pdfPage = p->getPage()->getPdfPageNr();

	int pos = p->getY();

	control->firePageSelected(pageNo);

	y = y * control->getZoomControl()->getZoom();

	if (y == 0) {
		y = -10; // show the shadow on top
	}

	double v = (double) pos + y;
	double upper = gtk_adjustment_get_upper(h) - gtk_adjustment_get_page_size(h);

	if (upper < v) {
		v = upper;
	}

	gtk_adjustment_set_value(h, v);
}

void XournalView::onScrolled() {
	GtkAdjustment * h = gtk_xournal_get_vadj(this->widget);
	GtkAllocation allocation = { 0 };
	gtk_widget_get_allocation(this->widget, &allocation);

	int scrollY = gtk_adjustment_get_value(h);

	int viewHeight = allocation.height;
	bool twoPages = control->getSettings()->isShowTwoPages();

	if (scrollY < 1) {
		if (twoPages && this->viewPagesLen > 1 && this->viewPages[1]->isSelected()) {
			// page 2 already selected
		} else {
			this->control->firePageSelected(0);
		}
		return;
	}

	int mostPageNr = 0;
	double mostPagePercent = 0;

	// next four pages are not marked as invisible,
	// because usually you scroll forward

	int visibelPageAdd = 4;

	for (int page = 0; page < this->viewPagesLen; page++) {
		PageView * p = this->viewPages[page];
		int y = p->getY();

		int pageHeight = p->getDisplayHeight();

		if (y > scrollY + viewHeight) {
			p->setIsVisibel(false);
			for (; page < this->viewPagesLen; page++) {
				p = this->viewPages[page];
				p->setIsVisibel(false);
			}

			break;
		}
		if (y + pageHeight >= scrollY) {
			int startY = 0;
			int endY = pageHeight;
			if (y <= scrollY) {
				startY = scrollY - y;
			}
			if (y + pageHeight > scrollY + viewHeight) {
				endY = pageHeight - ((y + pageHeight) - (scrollY + viewHeight));
			}

			double percent = ((double) (endY - startY)) / ((double) pageHeight);

			if (percent > mostPagePercent) {
				mostPagePercent = percent;
				mostPageNr = page;
			}

			p->setIsVisibel(true);
		} else {
			p->setIsVisibel(false);
		}
	}

	if (twoPages && mostPageNr < this->viewPagesLen - 1) {
		int y1 = this->viewPages[mostPageNr]->getY();
		int y2 = this->viewPages[mostPageNr + 1]->getY();

		if (y1 != y2 || !this->viewPages[mostPageNr + 1]->isSelected()) {
			// if the second page is selected DON'T select the first page.
			// Only select the first page if none is selected
			this->control->firePageSelected(mostPageNr);
		}
	} else {
		this->control->firePageSelected(mostPageNr);
	}
}

void XournalView::endTextSelection() {
	for (int i = 0; i < this->viewPagesLen; i++) {
		PageView * v = this->viewPages[i];
		v->endText();
	}
}

void XournalView::layerChanged(int page) {
	if (page >= 0 && page < this->viewPagesLen) {
		this->viewPages[page]->rerenderPage();
	}
}

void XournalView::getPasteTarget(double & x, double & y) {
	int pageNo = getCurrentPage();
	if (pageNo == -1) {
		return;
	}

	Rectangle * rect = getVisibleRect(pageNo);

	if (rect) {
		x = rect->width / 2 + rect->x;
		y = rect->height / 2 + rect->y;
	}
}
/**
 * Return the rectangle which is visible on screen, in document cooordinates
 *
 * Or NULL if the page is not visible
 */
Rectangle * XournalView::getVisibleRect(int page) {
	if (page < 0 || page >= this->viewPagesLen) {
		return NULL;
	}

	GtkAllocation allocation = { 0 };
	gtk_widget_get_allocation(this->widget, &allocation);

	GtkAdjustment * v = gtk_xournal_get_vadj(this->widget);
	int scrollY = gtk_adjustment_get_value(v);
	GtkAdjustment * h = gtk_xournal_get_hadj(this->widget);
	int scrollX = gtk_adjustment_get_value(h);

	int viewHeight = allocation.height;
	int viewWidth = allocation.width;

	PageView * p = this->viewPages[page];

	int posY = p->getY();
	int posX = p->getX();
	int pageHeight = p->getDisplayHeight();
	int pageWidth = p->getDisplayWidth();

	// page is after visible area
	if (scrollY + viewHeight < posY) {
		return NULL;
	}

	// page is before visible area
	if (posY + pageHeight < scrollY) {
		return NULL;
	}

	// page is after visible area
	if (scrollX + viewWidth < posX) {
		return NULL;
	}

	// page is before visible area
	if (posX + pageWidth < scrollX) {
		return NULL;
	}

	double y = scrollY - posY;
	double x = scrollX - posX;
	double height = viewHeight + y - pageHeight;
	double width = viewWidth + x - pageWidth;

	double zoom = getZoom();

	Rectangle * rect = new Rectangle(MAX(x, 0) / zoom, MAX(y, 0) / zoom, width / zoom, height / zoom);

	return rect;
}

GtkWidget * XournalView::getWidget() {
	return widget;
}

void XournalView::zoomIn() {
	control->getZoomControl()->zoomIn();
}

void XournalView::zoomOut() {
	control->getZoomControl()->zoomOut();
}

void XournalView::ensureRectIsVisible(int x, int y, int width, int heigth) {
	gtk_xournal_ensure_rect_is_visible(this->widget, x, y, width, heigth);
}

void XournalView::zoomChanged(double lastZoom) {
	GtkAdjustment * h = gtk_xournal_get_vadj(this->widget);
	double scrollY = gtk_adjustment_get_value(h);

	layoutPages();

	double zoom = control->getZoomControl()->getZoom();
	gtk_adjustment_set_value(h, scrollY / lastZoom * control->getZoomControl()->getZoom());

	Document * doc = control->getDocument();
	doc->lock();
	String file = doc->getEvMetadataFilename();
	doc->unlock();

	control->getMetadataManager()->setDouble(file, "zoom", zoom);
}

void XournalView::pageSizeChanged(int page) {
	PageView * v = this->viewPages[page];
	layoutPages();
}

void XournalView::pageChanged(int page) {
	if (page >= 0 && page < this->viewPagesLen) {
		this->viewPages[page]->rerenderPage();
	}
}

void XournalView::pageDeleted(int page) {
	delete this->viewPages[page];
	for (int i = page; i < this->viewPagesLen; i++) {
		this->viewPages[i] = this->viewPages[i + 1];
	}

	this->viewPagesLen--;

	layoutPages();
}

TextEditor * XournalView::getTextEditor() {
	for (int i = 0; i < this->viewPagesLen; i++) {
		PageView * v = this->viewPages[i];
		if (v->getTextEditor()) {
			return v->getTextEditor();
		}
	}

	return NULL;
}

void XournalView::resetShapeRecognizer() {
	for (int i = 0; i < this->viewPagesLen; i++) {
		PageView * v = this->viewPages[i];
		v->resetShapeRecognizer();
	}
}

PdfCache * XournalView::getCache() {
	return this->cache;
}

void XournalView::pageInserted(int page) {
	PageView ** lastViewPages = this->viewPages;

	this->viewPages = new PageView *[this->viewPagesLen + 1];

	for (int i = 0; i < page; i++) {
		this->viewPages[i] = lastViewPages[i];

		// unselect to prevent problems...
		this->viewPages[i]->setSelected(false);
	}

	for (int i = page; i < this->viewPagesLen; i++) {
		this->viewPages[i + 1] = lastViewPages[i];

		// unselect to prevent problems...
		this->viewPages[i + 1]->setSelected(false);
	}

	this->lastSelectedPage = -1;

	this->viewPagesLen++;

	delete[] lastViewPages;

	Document * doc = control->getDocument();
	doc->lock();
	PageView * pageView = new PageView(this, doc->getPage(page));
	doc->unlock();

	this->viewPages[page] = pageView;

	layoutPages();

	// Update scroll info, so we can call isPageVisible after
	onScrolled();
}

double XournalView::getZoom() {
	return control->getZoomControl()->getZoom();
}

void XournalView::updateXEvents() {
	gtk_xournal_update_xevent(this->widget);
}

void XournalView::clearSelection() {
	delete GTK_XOURNAL(widget)->selection;
	GTK_XOURNAL(widget)->selection = NULL;

	control->setClipboardHandlerSelection(getSelection());

	getCursor()->setMouseSelectionType(CURSOR_SELECTION_NONE);
	control->getToolHandler()->setSelectionEditTools(false, false);
}

void XournalView::deleteSelection() {

	// TODO ????????????? delete selection
	//	EditSelection * sel = getSelection();
	//	if (sel) {
	//		PageView * view = sel->getView();
	//		DeleteUndoAction * undo = new DeleteUndoAction(sel->getPage(), view, false);
	//		sel->fillUndoItem(undo);
	//		control->getUndoRedoHandler()->addUndoAction(undo);
	//
	//		sel->clearContents();
	//
	//		clearSelection();
	//
	//		view->rerenderPage();
	//	}
}

void XournalView::setSelection(EditSelection * selection) {
	clearSelection();
	GTK_XOURNAL(widget)->selection = selection;

	control->setClipboardHandlerSelection(getSelection());

	bool canChangeSize = false;
	bool canChangeColor = false;

	ListIterator<Element *> it = selection->getElements();

	while (it.hasNext()) {
		Element * e = it.next();
		if (e->getType() == ELEMENT_TEXT) {
			canChangeColor = true;
		} else if (e->getType() == ELEMENT_STROKE) {
			Stroke * s = (Stroke *) e;
			if (s->getToolType() != STROKE_TOOL_ERASER) {
				canChangeColor = true;
			}
			canChangeSize = true;
		}

		if (canChangeColor && canChangeSize) {
			break;
		}
	}

	control->getToolHandler()->setSelectionEditTools(canChangeColor, canChangeSize);

	repaintSelection();
}

void XournalView::repaintSelection() {
	EditSelection * selection = getSelection();
	if (selection == NULL) {
		return;
	}

	//	Redrawable * red = selection->getView();
	//	double zoom = getZoom();
	//	int x0 = red->getX();
	//	int y0 = red->getY();
	//	int x = selection->getX() * zoom;
	//	int y = selection->getY() * zoom;
	//	int w = selection->getWidth() * zoom;
	//	int h = selection->getHeight() * zoom;
	//
	//	gtk_xournal_repaint_area(this->widget, x0 + x - 10, y0 + y - 10, w + 20, h + 20);

	// TODO: OPTIMIZE ?
	gtk_widget_queue_draw(this->widget);
}

void XournalView::layoutPages() {
	GtkAllocation alloc = { 0 };

	gtk_widget_get_allocation(this->widget, &alloc);

	Settings * settings = getControl()->getSettings();

	bool showTwoPages = settings->isShowTwoPages();

	bool allowScrollOutsideThePage = settings->isAllowScrollOutsideThePage();

	int width = alloc.width;
	int height = XOURNAL_PADDING_TOP_LEFT;

	if (showTwoPages) {
		// TODO LOW PRIO: handle single landscape page better
		// If there is a landscape page, display them on a single line, not with another page

		// calc size for the widget
		for (int i = 0; i < this->viewPagesLen; i++) {
			int w = this->viewPages[i]->getDisplayWidth() + XOURNAL_PADDING_TOP_LEFT + XOURNAL_PADDING;
			int h = this->viewPages[i]->getDisplayHeight();
			if (i < this->viewPagesLen - 1) {

				i++;
				w += this->viewPages[i]->getDisplayWidth();
				w += XOURNAL_PADDING;
				h = MAX(h, this->viewPages[i]->getDisplayHeight());
			}
			if (width < w) {
				width = w;
			}
			height += h;
			height += XOURNAL_PADDING;
		}

		int y = XOURNAL_PADDING_TOP_LEFT;

		int additionalHeight = 0;
		if (viewPagesLen > 0) {
			additionalHeight = this->viewPages[0]->getHeight();
		}

		if (allowScrollOutsideThePage && viewPagesLen > 0) {
			y += this->viewPages[0]->getHeight() / 2;
			height += additionalHeight;
			width *= 2;
		}

		// layout pages
		for (int i = 0; i < viewPagesLen; i++) {
			int x = 0;
			int h = this->viewPages[i]->getDisplayHeight();
			if (i < this->viewPagesLen - 1) {
				x = width - this->viewPages[i]->getDisplayWidth() - this->viewPages[i + 1]->getDisplayWidth() - XOURNAL_PADDING - XOURNAL_PADDING_TOP_LEFT;
				x /= 2;

				this->viewPages[i]->setPos(x, y);

				x += this->viewPages[i]->getDisplayWidth() + XOURNAL_PADDING;

				i++;

				h = MAX(h, this->viewPages[i]->getDisplayHeight());
			} else {
				x = width - this->viewPages[i]->getDisplayWidth();
				x /= 2;
			}

			this->viewPages[i]->setPos(x, y);
			y += h;
			y += XOURNAL_PADDING;
		}

		gtk_xournal_set_size(this->widget, width, height);

		if (allowScrollOutsideThePage) {
			GtkAdjustment * hadj = gtk_xournal_get_hadj(this->widget);
			gtk_adjustment_set_value(hadj, width / 4);

			GtkAdjustment * vadj = gtk_xournal_get_vadj(this->widget);
			gtk_adjustment_set_value(vadj, additionalHeight / 2);
		}
	} else { // single page
		// calc size for the widget
		for (int i = 0; i < this->viewPagesLen; i++) {
			PageView * pageView = this->viewPages[i];
			int w = pageView->getDisplayWidth() + 20; // 20px for shadow

			if (width < w) {
				width = w;
			}
			height += pageView->getDisplayHeight();
			height += XOURNAL_PADDING;
		}

		int y = XOURNAL_PADDING_TOP_LEFT;
		int x = XOURNAL_PADDING_TOP_LEFT;

		int additionalHeight = 0;
		if (this->viewPagesLen > 0) {
			additionalHeight = this->viewPages[0]->getHeight();
		}

		if (allowScrollOutsideThePage && this->viewPagesLen > 0) {
			y += this->viewPages[0]->getHeight() / 2;
			height += additionalHeight;
			width *= 2;
		}

		gtk_xournal_set_size(this->widget, width, height);

		// layout pages
		for (int i = 0; i < this->viewPagesLen; i++) {
			PageView * pageView = this->viewPages[i];

			x = width - pageView->getDisplayWidth();
			x /= 2;

			pageView->setPos(x, y);
			y += pageView->getDisplayHeight();
			y += XOURNAL_PADDING;
		}

		if (allowScrollOutsideThePage) {
			GtkAdjustment * hadj = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(gtk_widget_get_parent(this->widget)));
			gtk_adjustment_set_value(hadj, width / 4);

			GtkAdjustment * vadj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(gtk_widget_get_parent(this->widget)));
			gtk_adjustment_set_value(vadj, additionalHeight / 2);
		}
	}

	gtk_widget_queue_draw(this->widget);

	this->pagePosition->update(this->viewPages, this->viewPagesLen, height);
}

bool XournalView::isPageVisible(int page) {
	Rectangle * rect = getVisibleRect(page);
	if (rect) {
		delete rect;
		return true;
	}

	return false;
}

void XournalView::documentChanged(DocumentChangeType type) {
	if (type != DOCUMENT_CHANGE_CLEARED && type != DOCUMENT_CHANGE_COMPLETE) {
		return;
	}
	for (int i = 0; i < viewPagesLen; i++) {
		delete viewPages[i];
	}
	delete[] viewPages;

	Document * doc = control->getDocument();
	doc->lock();

	this->viewPagesLen = doc->getPageCount();
	this->viewPages = new PageView*[viewPagesLen];

	for (int i = 0; i < viewPagesLen; i++) {
		PageView * pageView = new PageView(this, doc->getPage(i));
		viewPages[i] = pageView;
	}

	doc->unlock();

	layoutPages();
}

bool XournalView::cut() {
	int p = getCurrentPage();
	if (p < 0 || p >= viewPagesLen) {
		return false;
	}

	PageView * page = viewPages[p];
	return page->cut();
}

bool XournalView::copy() {
	int p = getCurrentPage();
	if (p < 0 || p >= viewPagesLen) {
		return false;
	}

	PageView * page = viewPages[p];
	return page->copy();
}

bool XournalView::paste() {
	int p = getCurrentPage();
	if (p < 0 || p >= viewPagesLen) {
		return false;
	}

	PageView * page = viewPages[p];
	return page->paste();
}

bool XournalView::actionDelete() {
	int p = getCurrentPage();
	if (p < 0 || p >= viewPagesLen) {
		return false;
	}

	PageView * page = viewPages[p];
	return page->actionDelete();
}

Document * XournalView::getDocument() {
	return control->getDocument();
}

ArrayIterator<PageView *> XournalView::pageViewIterator() {
	return ArrayIterator<PageView *> (viewPages, viewPagesLen);
}

PagePositionHandler * XournalView::getPagePositionHandler() {
	return this->pagePosition;
}

Cursor * XournalView::getCursor() {
	return control->getCursor();
}

EditSelection * XournalView::getSelection() {
	return GTK_XOURNAL(widget)->selection;
}

int XournalView::getMaxAreaX() {
	return GTK_XOURNAL(widget)->width;
}

int XournalView::getMaxAreaY() {
	return GTK_XOURNAL(widget)->height;
}

