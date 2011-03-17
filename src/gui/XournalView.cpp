#include "XournalView.h"
#include <gdk/gdkkeysyms.h>
#include "../control/Control.h"
#include <math.h>
#include "Shadow.h"
#include "../util/Util.h"

#include "../model/Document.h"
#include "PageView.h"
#include "../control/PdfCache.h"
#include "../control/settings/MetadataManager.h"
#include "../util/Rectangle.h"
#include "widgets/XournalWidget.h"

// TODO: LOW PRIO handle scroll events from touch / if Hand tool is selected

XournalView::XournalView(GtkWidget * parent, GtkRange * hrange, GtkRange * vrange, Control * control) {
	this->control = control;
	this->cache = new PdfCache(control->getSettings()->getPdfPageCacheSize());
	registerListener(control);

	initScrollHandler(parent);

	this->widget = gtk_xournal_new(this, hrange, vrange);

	gtk_table_attach_defaults(GTK_TABLE(parent), this->widget, 0, 1, 0, 1);
	gtk_widget_show(this->widget);

	this->viewPages = NULL;
	this->viewPagesLen = 0;
	this->margin = 75;
	this->currentPage = 0;
	this->lastSelectedPage = -1;

	// TODO: !!!!!!!!!!!!!!!!!!
	//	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(parent), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	//	g_signal_connect(this->widget, "size-allocate", G_CALLBACK(sizeAllocate), this);
	//	g_signal_connect(this->widget, "button_press_event", G_CALLBACK(onButtonPressEventCallback), this);
	//	gtk_widget_set_events(this->widget, GDK_BUTTON_PRESS_MASK);
	//
	//	g_signal_connect(G_OBJECT(this->widget), "expose_event", G_CALLBACK(exposeEventCallback), this);
	//
	//	GtkAdjustment* adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(parent));
	//	g_signal_connect(adj, "value-changed", G_CALLBACK(onVscrollChanged), this);
	//
	//	gtk_widget_set_events(this->widget, GDK_EXPOSURE_MASK | GDK_KEY_PRESS_MASK);
	//	gtk_signal_connect(GTK_OBJECT(this->widget), "key_press_event", GTK_SIGNAL_FUNC(onKeyPressCallback), this);
	//	gtk_signal_connect(GTK_OBJECT(this->widget), "key_release_event", GTK_SIGNAL_FUNC(onKeyReleaseCallback), this);

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

void XournalView::onVscrollChanged(GtkAdjustment *adjustment, XournalView * xournal) {
	xournal->onScrolled();
}

int XournalView::getCurrentPage() {
	return currentPage;
}

bool XournalView::onKeyPressCallback(GtkWidget *widget, GdkEventKey *event, XournalView * xournal) {
	return xournal->onKeyPressEvent(widget, event);
}

bool XournalView::onKeyReleaseCallback(GtkWidget *widget, GdkEventKey *event, XournalView * xournal) {
	return xournal->onKeyReleaseEvent(event);
}

const int scrollKeySize = 10;

bool XournalView::onKeyPressEvent(GtkWidget *widget, GdkEventKey *event) {
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
			control->getScrollHandler()->scrollRelative(0, windowHeight);
			return true;
		}
		if (event->keyval == GDK_Page_Up) {
			control->getScrollHandler()->scrollRelative(0, -windowHeight);
			return true;
		}
	}

	if (event->keyval == GDK_Up) {
		control->getScrollHandler()->scrollRelative(0, -scrollKeySize);
		return true;
	}

	if (event->keyval == GDK_Down) {
		control->getScrollHandler()->scrollRelative(0, scrollKeySize);
		return true;
	}

	if (event->keyval == GDK_Left) {
		control->getScrollHandler()->scrollRelative(-scrollKeySize, 0);
		return true;
	}

	if (event->keyval == GDK_Right) {
		control->getScrollHandler()->scrollRelative(scrollKeySize, 0);
		return true;
	}

	return false;
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

PageView * XournalView::getViewAt(int x, int y) {
	// TODO: low prio: binary search

	for (int page = 0; page < this->viewPagesLen; page++) {
		PageView * p = this->viewPages[page];
		if(p->containsPoint(x, y)) {
			return p;
		}
	}

	return NULL;
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
	// TODO: !!!!!!!!!!!!!!!!!!!!!!!

	//	if (currentPage == pageNo) {
	//		return;
	//	}
	//	if (pageNo < 0 || pageNo >= viewPagesLen) {
	//		return;
	//	}
	//	// TODO LOW PRIO: handle horizontal scrolling (dual page view)
	//
	//	GtkAdjustment* h = gtk_layout_get_vadjustment(GTK_LAYOUT(widget));
	//
	//	PageView * p = viewPages[pageNo];
	//
	//	int pdfPage = p->getPage()->getPdfPageNr();
	//
	//	GValue pY = { 0 };
	//	g_value_init(&pY, G_TYPE_INT);
	//
	//	gtk_container_child_get_property(GTK_CONTAINER(widget), p->getWidget(), "y", &pY);
	//	int pos = g_value_get_int(&pY);
	//
	//	control->firePageSelected(pageNo);
	//
	//	y = y * control->getZoomControl()->getZoom();
	//
	//	if (y == 0) {
	//		y = -10; // show the shadow on top
	//	}
	//
	//	double v = (double) pos + y;
	//	double upper = gtk_adjustment_get_upper(h) - gtk_adjustment_get_page_size(h);
	//
	//	if (upper < v) {
	//		v = upper;
	//	}
	//
	//	gtk_adjustment_set_value(h, v);
}

void XournalView::onScrolled() {
	// TODO!!!!!!!!!!!!!!
	//	GtkAdjustment * h = gtk_layout_get_vadjustment(GTK_LAYOUT(widget));
	//
	//	GtkAllocation allocation = { 0 };
	//	GtkWidget * w = gtk_widget_get_parent(widget);
	//	gtk_widget_get_allocation(w, &allocation);
	//
	//	int scrollY = gtk_adjustment_get_value(h);
	//
	//	int viewHeight = allocation.height;
	//
	//	bool twoPages = control->getSettings()->isShowTwoPages();
	//
	//	if (scrollY < 1) {
	//		if (twoPages && viewPagesLen > 1 && viewPages[1]->isSelected()) {
	//			// page 2 already selected
	//		} else {
	//			control->firePageSelected(0);
	//		}
	//		return;
	//	}
	//
	//	int mostPageNr = 0;
	//	double mostPagePercent = 0;
	//
	//	// next four pages are not marked as invisible,
	//	// because usually you scroll forward
	//
	//	int visibelPageAdd = 4;
	//
	//	for (int page = 0; page < viewPagesLen; page++) {
	//		PageView * p = viewPages[page];
	//		GValue pY = { 0 };
	//		g_value_init(&pY, G_TYPE_INT);
	//
	//		gtk_container_child_get_property(GTK_CONTAINER(widget), p->getWidget(), "y", &pY);
	//		int y = g_value_get_int(&pY);
	//
	//		int pageHeight = p->getDisplayHeight();
	//
	//		if (y > scrollY + viewHeight) {
	//			p->setIsVisibel(false);
	//			for (; page < viewPagesLen; page++) {
	//				p = viewPages[page];
	//				p->setIsVisibel(false);
	//			}
	//
	//			break;
	//		}
	//		if (y + pageHeight >= scrollY) {
	//			int startY = 0;
	//			int endY = pageHeight;
	//			if (y <= scrollY) {
	//				startY = scrollY - y;
	//			}
	//			if (y + pageHeight > scrollY + viewHeight) {
	//				endY = pageHeight - ((y + pageHeight) - (scrollY + viewHeight));
	//			}
	//
	//			double percent = ((double) (endY - startY)) / ((double) pageHeight);
	//
	//			if (percent > mostPagePercent) {
	//				mostPagePercent = percent;
	//				mostPageNr = page;
	//			}
	//
	//			p->setIsVisibel(true);
	//		} else {
	//			p->setIsVisibel(false);
	//		}
	//	}
	//
	//	if (twoPages && mostPageNr < viewPagesLen - 1) {
	//		GValue pY1 = { 0 };
	//		g_value_init(&pY1, G_TYPE_INT);
	//		GValue pY2 = { 0 };
	//		g_value_init(&pY2, G_TYPE_INT);
	//
	//		gtk_container_child_get_property(GTK_CONTAINER(widget), viewPages[mostPageNr]->getWidget(), "y", &pY1);
	//		int y1 = g_value_get_int(&pY1);
	//		gtk_container_child_get_property(GTK_CONTAINER(widget), viewPages[mostPageNr + 1]->getWidget(), "y", &pY2);
	//		int y2 = g_value_get_int(&pY2);
	//
	//		if (y1 != y2 || !viewPages[mostPageNr + 1]->isSelected()) {
	//			// if the second page is selected DON'T select the first page.
	//			// Only select the first page if none is selected
	//			control->firePageSelected(mostPageNr);
	//		}
	//	} else {
	//		control->firePageSelected(mostPageNr);
	//	}
}

void XournalView::endTextSelection() {
	for (int i = 0; i < viewPagesLen; i++) {
		PageView * v = viewPages[i];
		v->endText();
	}
}

void XournalView::layerChanged(int page) {
	viewPages[page]->repaint();
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
	// TODO !!!!!!!!!!!!!!!!!!!
	//	if (page < 0 || page >= this->viewPagesLen) {
	//		return NULL;
	//	}
	//
	//	GtkAllocation allocation = { 0 };
	//	GtkWidget * w = gtk_widget_get_parent(widget);
	//	gtk_widget_get_allocation(w, &allocation);
	//
	//	GtkAdjustment * v = gtk_layout_get_vadjustment(GTK_LAYOUT(widget));
	//	int scrollY = gtk_adjustment_get_value(v);
	//	GtkAdjustment * h = gtk_layout_get_hadjustment(GTK_LAYOUT(widget));
	//	int scrollX = gtk_adjustment_get_value(h);
	//
	//	int viewHeight = allocation.height;
	//	int viewWidth = allocation.width;
	//
	//	PageView * p = viewPages[page];
	//
	//	GValue gValue = { 0 };
	//	g_value_init(&gValue, G_TYPE_INT);
	//
	//	gtk_container_child_get_property(GTK_CONTAINER(widget), p->getWidget(), "y", &gValue);
	//	int posY = g_value_get_int(&gValue);
	//
	//	gtk_container_child_get_property(GTK_CONTAINER(widget), p->getWidget(), "x", &gValue);
	//	int posX = g_value_get_int(&gValue);
	//
	//	gtk_widget_get_allocation(p->getWidget(), &allocation);
	//	int pageHeight = allocation.height;
	//	int pageWidth = allocation.width;
	//
	//	// page is after visible area
	//	if (scrollY + viewHeight < posY) {
	//		return NULL;
	//	}
	//
	//	// page is before visible area
	//	if (posY + pageHeight < scrollY) {
	//		return NULL;
	//	}
	//
	//	// page is after visible area
	//	if (scrollX + viewWidth < posX) {
	//		return NULL;
	//	}
	//
	//	// page is before visible area
	//	if (posX + pageWidth < scrollX) {
	//		return NULL;
	//	}
	//
	//	double y = scrollY - posY;
	//	double x = scrollX - posX;
	//	double height = viewHeight + y - pageHeight;
	//	double width = viewWidth + x - pageWidth;
	//	Rectangle * rect = new Rectangle(MAX(x, 0) / getZoom(), MAX(y, 0) / getZoom(), width / getZoom(), height / getZoom());
	//
	//	return rect;

	return NULL;
}

GtkWidget * XournalView::getWidget() {
	// TODO: !!!!!!!!!!!
	return widget;
}

void XournalView::initScrollHandler(GtkWidget * parent) {

	// TODO: !!!!!!!!!!!!!!This is not working anymore

	GtkBindingSet * bindingSet = gtk_binding_set_by_class(G_OBJECT_GET_CLASS(parent));
	gtk_binding_entry_add_signal(bindingSet, GDK_Up, (GdkModifierType) 0, "scroll_child", 2, GTK_TYPE_SCROLL_TYPE, GTK_SCROLL_STEP_BACKWARD, G_TYPE_BOOLEAN,
			FALSE);
	gtk_binding_entry_add_signal(bindingSet, GDK_KP_Up, (GdkModifierType) 0, "scroll_child", 2, GTK_TYPE_SCROLL_TYPE, GTK_SCROLL_STEP_BACKWARD, G_TYPE_BOOLEAN,
			FALSE);
	gtk_binding_entry_add_signal(bindingSet, GDK_Down, (GdkModifierType) 0, "scroll_child", 2, GTK_TYPE_SCROLL_TYPE, GTK_SCROLL_STEP_FORWARD, G_TYPE_BOOLEAN,
			FALSE);
	gtk_binding_entry_add_signal(bindingSet, GDK_KP_Down, (GdkModifierType) 0, "scroll_child", 2, GTK_TYPE_SCROLL_TYPE, GTK_SCROLL_STEP_FORWARD,
			G_TYPE_BOOLEAN, FALSE);
	gtk_binding_entry_add_signal(bindingSet, GDK_Left, (GdkModifierType) 0, "scroll_child", 2, GTK_TYPE_SCROLL_TYPE, GTK_SCROLL_STEP_BACKWARD, G_TYPE_BOOLEAN,
			TRUE);
	gtk_binding_entry_add_signal(bindingSet, GDK_KP_Left, (GdkModifierType) 0, "scroll_child", 2, GTK_TYPE_SCROLL_TYPE, GTK_SCROLL_STEP_BACKWARD,
			G_TYPE_BOOLEAN, TRUE);
	gtk_binding_entry_add_signal(bindingSet, GDK_Right, (GdkModifierType) 0, "scroll_child", 2, GTK_TYPE_SCROLL_TYPE, GTK_SCROLL_STEP_FORWARD, G_TYPE_BOOLEAN,
			TRUE);
	gtk_binding_entry_add_signal(bindingSet, GDK_KP_Right, (GdkModifierType) 0, "scroll_child", 2, GTK_TYPE_SCROLL_TYPE, GTK_SCROLL_STEP_FORWARD,
			G_TYPE_BOOLEAN, TRUE);
}

void XournalView::zoomIn() {
	control->getZoomControl()->zoomIn();
}

void XournalView::zoomOut() {
	control->getZoomControl()->zoomOut();
}

void XournalView::zoomChanged(double lastZoom) {
	// TODO !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	//	GtkAdjustment* h = gtk_layout_get_vadjustment(GTK_LAYOUT(widget));
	//	GtkAllocation allocation = { 0 };
	//	GtkWidget * w = gtk_widget_get_parent(widget);
	//	gtk_widget_get_allocation(w, &allocation);
	//	double scrollY = gtk_adjustment_get_value(h);
	//
	//	layoutPages();
	//
	//	for (int i = 0; i < viewPagesLen; i++) {
	//		PageView * pageView = viewPages[i];
	//		pageView->updateSize();
	//	}
	//
	//	double zoom = control->getZoomControl()->getZoom();
	//	gtk_adjustment_set_value(h, scrollY / lastZoom * control->getZoomControl()->getZoom());
	//
	//	Document * doc = control->getDocument();
	//	doc->lock();
	//	String file = doc->getEvMetadataFilename();
	//	doc->unlock();
	//
	//	control->getMetadataManager()->setDouble(file, "zoom", zoom);
}

void XournalView::pageSizeChanged(int page) {
	PageView * v = this->viewPages[page];
	layoutPages();
}

void XournalView::pageChanged(int page) {
	if (page >= 0 && page < this->viewPagesLen) {
		this->viewPages[page]->repaint();
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

	// TODO !!!!!!!! still neccessary?
	// Update scroll info, so we can call isPageVisible after
	onScrolled();
}

double XournalView::getZoom() {
	return control->getZoomControl()->getZoom();
}

void XournalView::updateXEvents() {
	gtk_xournal_update_xevent(this->widget);
}

void XournalView::layoutPages() {
	GtkAllocation alloc = { 0 };

	gtk_widget_get_allocation(this->widget, &alloc);

	Settings * settings = getControl()->getSettings();

	bool showTwoPages = settings->isShowTwoPages();

	bool allowScrollOutsideThePage = settings->isAllowScrollOutsideThePage();

	if (showTwoPages) {
		int width = alloc.width;
		int height = XOURNAL_PADDING_TOP_LEFT;

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
				x = width - this->viewPages[i]->getDisplayWidth() - this->viewPages[i + 1]->getDisplayWidth() - XOURNAL_PADDING - +XOURNAL_PADDING_TOP_LEFT;
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
			GtkAdjustment * hadj = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(gtk_widget_get_parent(this->widget)));
			gtk_adjustment_set_value(hadj, width / 4);

			GtkAdjustment * vadj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(gtk_widget_get_parent(this->widget)));
			gtk_adjustment_set_value(vadj, additionalHeight / 2);
		}
	} else {
		int width = alloc.width;
		int height = XOURNAL_PADDING_TOP_LEFT;

		// calc size for the widget
		for (int i = 0; i < this->viewPagesLen; i++) {
			PageView * pageView = this->viewPages[i];
			int w = pageView->getDisplayWidth();
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

		for (int i = 0; i < viewPagesLen; i++) {
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

	//	TODO: !!!!!!!!! Need to redraw
	//	g_idle_add((GSourceFunc) widgetRepaintCallback, widget);
}

bool XournalView::isPageVisible(int page) {
	Rectangle * rect = getVisibleRect(page);
	if (rect) {
		delete rect;
		return true;
	}

	return false;
}

bool XournalView::widgetRepaintCallback(GtkWidget * widget) {
	gdk_threads_enter();
	gtk_widget_queue_draw(widget);
	gdk_threads_leave();
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
