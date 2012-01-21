#include "SidebarPreviews.h"
#include "SidebarPreviewPage.h"

#include "../../../control/Control.h"
#include "../../../control/PdfCache.h"

SidebarPreviews::SidebarPreviews(Control * control) :
	AbstractSidebarPage(control) {
	XOJ_INIT_TYPE(SidebarPreviews);

	this->previews = NULL;
	this->previewCount = 0;
	this->backgroundInitialized = false;

	this->zoom = 0.15;

	this->selectedPage = -1;

	this->cache = new PdfCache(control->getSettings()->getPdfPageCacheSize());

	this->iconViewPreview = gtk_layout_new(NULL, NULL);
	g_object_ref(this->iconViewPreview);

	this->scrollPreview = gtk_scrolled_window_new(NULL, NULL);
	g_object_ref(this->scrollPreview);

	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(this->scrollPreview), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(this->scrollPreview), GTK_SHADOW_IN);

	gtk_container_add(GTK_CONTAINER(this->scrollPreview), this->iconViewPreview);

	gtk_widget_show(this->iconViewPreview);

	registerListener(this->control);
}

SidebarPreviews::~SidebarPreviews() {
	XOJ_CHECK_TYPE(SidebarPreviews);

	gtk_widget_destroy(this->iconViewPreview);
	this->iconViewPreview = NULL;

	delete this->cache;
	this->cache = NULL;

	for (int i = 0; i < this->previewCount; i++) {
		delete this->previews[i];
	}
	delete[] this->previews;
	this->previewCount = 0;
	this->previews = NULL;

	XOJ_RELEASE_TYPE(SidebarPreviews);
}

void SidebarPreviews::setBackgroundWhite() {
	XOJ_CHECK_TYPE(SidebarPreviews);

	if (this->backgroundInitialized) {
		return;
	}
	this->backgroundInitialized = true;

	gdk_window_set_background(GTK_LAYOUT(this->iconViewPreview)->bin_window, &this->iconViewPreview->style->white);
}

double SidebarPreviews::getZoom() {
	XOJ_CHECK_TYPE(SidebarPreviews);

	return this->zoom;
}

PdfCache * SidebarPreviews::getCache() {
	XOJ_CHECK_TYPE(SidebarPreviews);

	return this->cache;
}

void SidebarPreviews::layout() {
	XOJ_CHECK_TYPE(SidebarPreviews);

	// TODO !!! extract layouting to class SidebarLayout

	int x = 0;
	int y = 0;
	int width = 0;

	for (int i = 0; i < this->previewCount; i++) {
		SidebarPreviewPage * p = this->previews[i];
		gtk_layout_move(GTK_LAYOUT(this->iconViewPreview), p->getWidget(), x, y);
		y += p->getHeight();
		width = MAX(width, p->getWidth());
	}

	gtk_layout_set_size(GTK_LAYOUT(this->iconViewPreview), width, y);
}

void SidebarPreviews::updatePreviews() {
	XOJ_CHECK_TYPE(SidebarPreviews);

	Document * doc = this->getControl()->getDocument();
	doc->lock();
	int len = doc->getPageCount();

	if (this->previewCount == len) {
		doc->unlock();
		return;
	}

	if (this->previews) {
		for (int i = 0; i < this->previewCount; i++) {
			delete this->previews[i];
		}
		delete[] this->previews;
	}

	this->previews = new SidebarPreviewPage *[len];
	this->previewCount = len;

	for (int i = 0; i < len; i++) {
		SidebarPreviewPage * p = new SidebarPreviewPage(this, doc->getPage(i));
		this->previews[i] = p;
		gtk_layout_put(GTK_LAYOUT(this->iconViewPreview), p->getWidget(), 0, 0);
	}

	layout();
	doc->unlock();
}

const char * SidebarPreviews::getName() {
	XOJ_CHECK_TYPE(SidebarPreviews);

	return _("Preview");
}

bool SidebarPreviews::hasData() {
	XOJ_CHECK_TYPE(SidebarPreviews);

	return true;
}

GtkWidget * SidebarPreviews::getWidget() {
	XOJ_CHECK_TYPE(SidebarPreviews);

	return this->scrollPreview;
}

void SidebarPreviews::documentChanged(DocumentChangeType type) {
	XOJ_CHECK_TYPE(SidebarPreviews);

	if (type == DOCUMENT_CHANGE_COMPLETE || type == DOCUMENT_CHANGE_CLEARED) {
		updatePreviews();
	}
}

void SidebarPreviews::pageSizeChanged(int page) {
	XOJ_CHECK_TYPE(SidebarPreviews);

	if (page < 0 || page >= this->previewCount) {
		return;
	}
	SidebarPreviewPage * p = this->previews[page];
	p->updateSize();
	p->repaint();

	layout();
}

void SidebarPreviews::pageChanged(int page) {
	XOJ_CHECK_TYPE(SidebarPreviews);

	if (page < 0 || page >= this->previewCount) {
		return;
	}

	SidebarPreviewPage * p = this->previews[page];
	p->repaint();
}

bool SidebarPreviews::scrollToPreview(SidebarPreviews * sidebar) {
	XOJ_CHECK_TYPE_OBJ(sidebar, SidebarPreviews);

	MainWindow * win = sidebar->control->getWindow();
	if (win) {
		GtkWidget * w = win->get("sidebarContents");
		if (!gtk_widget_get_visible(w)) {
			return false;
		}
	}

	if (sidebar->selectedPage >= 0 && sidebar->selectedPage < sidebar->previewCount) {
		gdk_threads_enter();
		SidebarPreviewPage * p = sidebar->previews[sidebar->selectedPage];

		// scroll to preview
		GtkAdjustment * hadj = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(sidebar->scrollPreview));
		GtkAdjustment * vadj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(sidebar->scrollPreview));
		GtkWidget * widget = p->getWidget();

		int x = widget->allocation.x;
		int y = widget->allocation.y;
		gdk_threads_leave();

		if (x == -1) {
			g_idle_add((GSourceFunc) scrollToPreview, sidebar);
			return false;
		}

		gdk_threads_enter();
		gtk_adjustment_clamp_page(vadj, y, y + widget->allocation.height);
		gtk_adjustment_clamp_page(hadj, x, x + widget->allocation.width);
		gdk_threads_leave();
	}
	return false;
}

void SidebarPreviews::pageDeleted(int page) {
	XOJ_CHECK_TYPE(SidebarPreviews);

	delete this->previews[page];
	for (int i = page; i < this->previewCount; i++) {
		this->previews[i] = this->previews[i + 1];
	}
	this->previewCount--;

	layout();
}

void SidebarPreviews::pageInserted(int page) {
	XOJ_CHECK_TYPE(SidebarPreviews);

	SidebarPreviewPage ** lastPreviews = this->previews;

	this->previews = new SidebarPreviewPage *[this->previewCount + 1];

	for (int i = 0; i < page; i++) {
		this->previews[i] = lastPreviews[i];

		// unselect to prevent problems...
		this->previews[i]->setSelected(false);
	}

	for (int i = page; i < this->previewCount; i++) {
		this->previews[i + 1] = lastPreviews[i];

		// unselect to prevent problems...
		this->previews[i + 1]->setSelected(false);
	}

	this->selectedPage = -1;

	this->previewCount++;

	delete[] lastPreviews;

	Document * doc = control->getDocument();
	doc->lock();

	SidebarPreviewPage * p = new SidebarPreviewPage(this, doc->getPage(page));

	doc->unlock();

	this->previews[page] = p;
	gtk_layout_put(GTK_LAYOUT(this->iconViewPreview), p->getWidget(), 0, 0);

	layout();
}

void SidebarPreviews::pageSelected(int page) {
	XOJ_CHECK_TYPE(SidebarPreviews);

	if (this->selectedPage >= 0 && this->selectedPage < this->previewCount) {
		this->previews[this->selectedPage]->setSelected(false);
	}
	this->selectedPage = page;

	if (this->selectedPage >= 0 && this->selectedPage < this->previewCount) {
		SidebarPreviewPage * p = this->previews[this->selectedPage];
		p->setSelected(true);
		scrollToPreview(this);
	}
}

