#include "XournalView.h"
#include "../control/Control.h"
#include <math.h>
#include "Shadow.h"
#include <Util.h>

#include "../model/Document.h"
#include "../model/Stroke.h"
#include "PageView.h"
#include "../control/PdfCache.h"
#include "../control/settings/MetadataManager.h"
#include <Rectangle.h>
#include "widgets/XournalWidget.h"
#include "pageposition/PagePositionHandler.h"
#include "Cursor.h"
#include "../undo/DeleteUndoAction.h"

#include "RepaintHandler.h"

#include <gdk/gdkkeysyms.h>

XournalView::XournalView(GtkWidget * parent, GtkRange * hrange, GtkRange * vrange, Control * control) {
	XOJ_INIT_TYPE(XournalView);

	this->control = control;
	this->cache = new PdfCache(control->getSettings()->getPdfPageCacheSize());
	registerListener(control);

	this->widget = gtk_xournal_new(this, hrange, vrange);

	// we need to refer widget here, because wo unref it somwere twice!?
	g_object_ref(this->widget);

	gtk_table_attach_defaults(GTK_TABLE(parent), this->widget, 1, 2, 0, 1);
	gtk_widget_show(this->widget);

	this->repaintHandler = new RepaintHandler(this);
	this->pagePosition = new PagePositionHandler();

	this->viewPages = NULL;
	this->viewPagesLen = 0;
	this->margin = 75;
	this->currentPage = 0;
	this->lastSelectedPage = -1;

	g_signal_connect(gtk_range_get_adjustment(vrange), "value-changed", G_CALLBACK(onVscrollChanged), this);

	control->getZoomControl()->addZoomListener(this);

	gtk_widget_set_can_default(this->widget, true);
	gtk_widget_grab_default(this->widget);

	gtk_widget_grab_focus(this->widget);

	this->cleanupTimeout = g_timeout_add_seconds(5, (GSourceFunc) clearMemoryTimer, this);
}

XournalView::~XournalView() {
	XOJ_CHECK_TYPE(XournalView);

	g_source_remove(this->cleanupTimeout);

	for (int i = 0; i < this->viewPagesLen; i++) {
		delete this->viewPages[i];
	}
	delete[] this->viewPages;
	this->viewPagesLen = 0;
	this->viewPages = NULL;

	delete this->cache;
	this->cache = NULL;
	delete this->repaintHandler;
	this->repaintHandler = NULL;

	delete this->pagePosition;
	this->pagePosition = NULL;

	this->widget = NULL;

	XOJ_RELEASE_TYPE(XournalView);
}

gint pageViewCmpSize(PageView * a, PageView * b) {
	return a->getLastVisibelTime() - b->getLastVisibelTime();
}

gboolean XournalView::clearMemoryTimer(XournalView * widget) {
	XOJ_CHECK_TYPE_OBJ(widget, XournalView);

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
	XOJ_CHECK_TYPE_OBJ(xournal, XournalView);

	xournal->onScrolled();
}

int XournalView::getCurrentPage() {
	XOJ_CHECK_TYPE(XournalView);

	return currentPage;
}

const int scrollKeySize = 10;

bool XournalView::onKeyPressEvent(GdkEventKey * event) {
	XOJ_CHECK_TYPE(XournalView);

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
	XOJ_CHECK_TYPE(XournalView);

	return this->repaintHandler;
}

bool XournalView::onKeyReleaseEvent(GdkEventKey * event) {
	XOJ_CHECK_TYPE(XournalView);

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
	XOJ_CHECK_TYPE(XournalView);

	gtk_widget_grab_focus(this->widget);
}

bool XournalView::searchTextOnPage(const char * text, int p, int * occures, double * top) {
	XOJ_CHECK_TYPE(XournalView);

	if (p < 0 || p >= this->viewPagesLen) {
		return false;
	}
	PageView * v = this->viewPages[p];

	return v->searchTextOnPage(text, occures, top);
}

void XournalView::forceUpdatePagenumbers() {
	XOJ_CHECK_TYPE(XournalView);

	int p = this->currentPage;
	this->currentPage = -1;

	control->firePageSelected(p);
}

PageView * XournalView::getViewFor(int pageNr) {
	XOJ_CHECK_TYPE(XournalView);

	if (pageNr < 0 || pageNr >= this->viewPagesLen) {
		return NULL;
	}
	return this->viewPages[pageNr];
}

void XournalView::pageSelected(int page) {
	XOJ_CHECK_TYPE(XournalView);

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
		pdfPage = vp->getPage().getPdfPageNr();
	}

	control->updatePageNumbers(currentPage, pdfPage);

	control->updateBackgroundSizeButton();
}

Control * XournalView::getControl() {
	XOJ_CHECK_TYPE(XournalView);

	return control;
}

void XournalView::scrollTo(int pageNo, double y) {
	XOJ_CHECK_TYPE(XournalView);

	if (this->currentPage == pageNo) {
		return;
	}
	if (pageNo < 0 || pageNo >= this->viewPagesLen) {
		return;
	}
	// TODO LOW PRIO: handle horizontal scrolling (dual page view)

	GtkAdjustment * h = gtk_xournal_get_vadj(this->widget);

	PageView * p = viewPages[pageNo];

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
	XOJ_CHECK_TYPE(XournalView);

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
	XOJ_CHECK_TYPE(XournalView);

	for (int i = 0; i < this->viewPagesLen; i++) {
		PageView * v = this->viewPages[i];
		v->endText();
	}
}

void XournalView::layerChanged(int page) {
	XOJ_CHECK_TYPE(XournalView);

	if (page >= 0 && page < this->viewPagesLen) {
		this->viewPages[page]->rerenderPage();
	}
}

void XournalView::getPasteTarget(double & x, double & y) {
	XOJ_CHECK_TYPE(XournalView);

	int pageNo = getCurrentPage();
	if (pageNo == -1) {
		return;
	}

	Rectangle * rect = getVisibleRect(pageNo);

	if (rect) {
		x = rect->width / 2;
		y = rect->height / 2;
	}
}
/**
 * Return the rectangle which is visible on screen, in document cooordinates
 *
 * Or NULL if the page is not visible
 */
Rectangle * XournalView::getVisibleRect(int page) {
	XOJ_CHECK_TYPE(XournalView);

	if (page < 0 || page >= this->viewPagesLen) {
		return NULL;
	}
	PageView * p = this->viewPages[page];

	return gtk_xournal_get_visible_area(this->widget, p);
}

GtkWidget * XournalView::getWidget() {
	XOJ_CHECK_TYPE(XournalView);

	return widget;
}

void XournalView::zoomIn() {
	XOJ_CHECK_TYPE(XournalView);

	control->getZoomControl()->zoomIn();
}

void XournalView::zoomOut() {
	XOJ_CHECK_TYPE(XournalView);

	control->getZoomControl()->zoomOut();
}

void XournalView::ensureRectIsVisible(int x, int y, int width, int heigth) {
	XOJ_CHECK_TYPE(XournalView);

	gtk_xournal_ensure_rect_is_visible(this->widget, x, y, width, heigth);
}

void XournalView::zoomChanged(double lastZoom) {
	XOJ_CHECK_TYPE(XournalView);

	GtkAdjustment * h = gtk_xournal_get_vadj(this->widget);
	double scrollY = gtk_adjustment_get_value(h);

	layoutPages();

	double zoom = control->getZoomControl()->getZoom();
	gtk_adjustment_set_value(h, scrollY / lastZoom * zoom);

	Document * doc = control->getDocument();
	doc->lock();
	String file = doc->getEvMetadataFilename();
	doc->unlock();

	control->getMetadataManager()->setDouble(file, "zoom", zoom);

	this->control->getScheduler()->blockRerenderZoom();
}

void XournalView::pageSizeChanged(int page) {
	XOJ_CHECK_TYPE(XournalView);
	layoutPages();
}

void XournalView::pageChanged(int page) {
	XOJ_CHECK_TYPE(XournalView);

	if (page >= 0 && page < this->viewPagesLen) {
		this->viewPages[page]->rerenderPage();
	}
}

void XournalView::pageDeleted(int page) {
	XOJ_CHECK_TYPE(XournalView);

	delete this->viewPages[page];
	for (int i = page; i < this->viewPagesLen; i++) {
		this->viewPages[i] = this->viewPages[i + 1];
	}

	this->viewPagesLen--;

	layoutPages();
}

TextEditor * XournalView::getTextEditor() {
	XOJ_CHECK_TYPE(XournalView);

	for (int i = 0; i < this->viewPagesLen; i++) {
		PageView * v = this->viewPages[i];
		if (v->getTextEditor()) {
			return v->getTextEditor();
		}
	}

	return NULL;
}

void XournalView::resetShapeRecognizer() {
	XOJ_CHECK_TYPE(XournalView);

	for (int i = 0; i < this->viewPagesLen; i++) {
		PageView * v = this->viewPages[i];
		v->resetShapeRecognizer();
	}
}

PdfCache * XournalView::getCache() {
	XOJ_CHECK_TYPE(XournalView);

	return this->cache;
}

void XournalView::pageInserted(int page) {
	XOJ_CHECK_TYPE(XournalView);

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
	XOJ_CHECK_TYPE(XournalView);

	return control->getZoomControl()->getZoom();
}

void XournalView::updateXEvents() {
	XOJ_CHECK_TYPE(XournalView);

	gtk_xournal_update_xevent(this->widget);
}

void XournalView::clearSelection() {
	XOJ_CHECK_TYPE(XournalView);

	EditSelection * sel = GTK_XOURNAL(widget)->selection;
	GTK_XOURNAL(widget)->selection = NULL;
	delete sel;

	control->setClipboardHandlerSelection(getSelection());

	getCursor()->setMouseSelectionType(CURSOR_SELECTION_NONE);
	control->getToolHandler()->setSelectionEditTools(false, false);
}

void XournalView::deleteSelection(EditSelection * sel) {
	XOJ_CHECK_TYPE(XournalView);

	if (sel == NULL) {
		sel = getSelection();
	}

	if (sel) {
		PageView * view = sel->getView();
		DeleteUndoAction * undo = new DeleteUndoAction(sel->getSourcePage(), view, false);
		sel->fillUndoItem(undo);
		control->getUndoRedoHandler()->addUndoAction(undo);

		clearSelection();

		view->rerenderPage();
		repaintSelection(true);
	}
}

void XournalView::setSelection(EditSelection * selection) {
	XOJ_CHECK_TYPE(XournalView);

	clearSelection();
	GTK_XOURNAL(this->widget)->selection = selection;

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

void XournalView::repaintSelection(bool evenWithoutSelection) {
	XOJ_CHECK_TYPE(XournalView);

	if (evenWithoutSelection) {
		gtk_widget_queue_draw(this->widget);
		return;
	}

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

	// TODO OPTIMIZE ?
	gtk_widget_queue_draw(this->widget);
}

void XournalView::layoutPages() {
	XOJ_CHECK_TYPE(XournalView);

	GtkAllocation alloc = { 0 };

	gtk_widget_get_allocation(this->widget, &alloc);

	Settings * settings = getControl()->getSettings();

	bool showTwoPages = settings->isShowTwoPages();

	bool allowScrollOutsideThePage = settings->isAllowScrollOutsideThePage();

	int width = alloc.width;
	int height = XOURNAL_PADDING_TOP_LEFT;

	int additionalHeight = 0;

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

		if (viewPagesLen > 0) {
			additionalHeight = this->viewPages[0]->getDisplayHeight();
		}

		if (allowScrollOutsideThePage && viewPagesLen > 0) {
			y += this->viewPages[0]->getDisplayHeight() / 2;
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

		if (this->viewPagesLen > 0) {
			additionalHeight = this->viewPages[0]->getDisplayHeight();
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
	}

	this->pagePosition->update(this->viewPages, this->viewPagesLen, height);

	gtk_widget_queue_draw(this->widget);
	
	// TODO low prio: scrol on right position on start
//	if (allowScrollOutsideThePage) {
//		GtkAdjustment * hadj = gtk_xournal_get_hadj(this->widget);
//		gtk_adjustment_set_value(hadj, width / 4);
//
//		GtkAdjustment * vadj = gtk_xournal_get_vadj(this->widget);
//		gtk_adjustment_set_value(vadj, additionalHeight / 2 - gtk_adjustment_get_page_size(vadj) + 20);
//	}
}

bool XournalView::isPageVisible(int page, int * visibleHeight) {
	XOJ_CHECK_TYPE(XournalView);

	Rectangle * rect = getVisibleRect(page);
	if (rect) {
		if (visibleHeight) {
			*visibleHeight = rect->height;
		}

		delete rect;
		return true;
	}
	if (visibleHeight) {
		*visibleHeight = 0;
	}

	return false;
}

void XournalView::documentChanged(DocumentChangeType type) {
	XOJ_CHECK_TYPE(XournalView);

	if (type != DOCUMENT_CHANGE_CLEARED && type != DOCUMENT_CHANGE_COMPLETE) {
		return;
	}

	XournalScheduler * scheduler = this->control->getScheduler();
	scheduler->lock();
	scheduler->removeAllJobs();

	clearSelection();

	for (int i = 0; i < this->viewPagesLen; i++) {
		delete this->viewPages[i];
	}
	delete[] this->viewPages;

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

	scheduler->unlock();
}

bool XournalView::cut() {
	XOJ_CHECK_TYPE(XournalView);

	int p = getCurrentPage();
	if (p < 0 || p >= viewPagesLen) {
		return false;
	}

	PageView * page = viewPages[p];
	return page->cut();
}

bool XournalView::copy() {
	XOJ_CHECK_TYPE(XournalView);

	int p = getCurrentPage();
	if (p < 0 || p >= viewPagesLen) {
		return false;
	}

	PageView * page = viewPages[p];
	return page->copy();
}

bool XournalView::paste() {
	XOJ_CHECK_TYPE(XournalView);

	int p = getCurrentPage();
	if (p < 0 || p >= viewPagesLen) {
		return false;
	}

	PageView * page = viewPages[p];
	return page->paste();
}

bool XournalView::actionDelete() {
	XOJ_CHECK_TYPE(XournalView);

	int p = getCurrentPage();
	if (p < 0 || p >= viewPagesLen) {
		return false;
	}

	PageView * page = viewPages[p];
	return page->actionDelete();
}

Document * XournalView::getDocument() {
	XOJ_CHECK_TYPE(XournalView);

	return control->getDocument();
}

ArrayIterator<PageView *> XournalView::pageViewIterator() {
	XOJ_CHECK_TYPE(XournalView);

	return ArrayIterator<PageView *> (viewPages, viewPagesLen);
}

PagePositionHandler * XournalView::getPagePositionHandler() {
	XOJ_CHECK_TYPE(XournalView);

	return this->pagePosition;
}

Cursor * XournalView::getCursor() {
	XOJ_CHECK_TYPE(XournalView);

	return control->getCursor();
}

EditSelection * XournalView::getSelection() {
	XOJ_CHECK_TYPE(XournalView);

	g_return_val_if_fail(this->widget != NULL, NULL);
	g_return_val_if_fail(GTK_IS_XOURNAL(this->widget), NULL);

	return GTK_XOURNAL(this->widget)->selection;
}

int XournalView::getMaxAreaX() {
	XOJ_CHECK_TYPE(XournalView);

	g_return_val_if_fail(this->widget != NULL, 0);
	g_return_val_if_fail(GTK_IS_XOURNAL(this->widget), 0);

	return GTK_XOURNAL(this->widget)->width;
}

int XournalView::getMaxAreaY() {
	XOJ_CHECK_TYPE(XournalView);

	g_return_val_if_fail(this->widget != NULL, 0);
	g_return_val_if_fail(GTK_IS_XOURNAL(this->widget), 0);

	return GTK_XOURNAL(this->widget)->height;
}

