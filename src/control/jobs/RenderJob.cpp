#include "RenderJob.h"
#include <gtk/gtk.h>
#include "../../gui/XournalWidget.h"
#include "../../view/PdfView.h"
#include "../../view/DocumentView.h"

RenderJob::RenderJob(PageView * view) {
	this->view = view;
}

RenderJob::~RenderJob() {
	this->view = NULL;
}

void * RenderJob::getSource() {
	return this->view;
}

void RenderJob::repaintRectangle(Rectangle * rect, double zoom) {
	XojPopplerPage * popplerPage = NULL;

	Document * doc = this->view->xournal->getDocument();
	doc->lock();

	if (this->view->page->getBackgroundType() == BACKGROUND_TYPE_PDF) {
		int pgNo = this->view->page->getPdfPageNr();
		popplerPage = doc->getPdfPage(pgNo);
	}

	double pageWidth = this->view->page->getWidth();
	double pageHeight = this->view->page->getHeight();

	doc->unlock();

	int x = rect->x * zoom;
	int y = rect->y * zoom;
	int width = rect->width * zoom;
	int height = rect->height * zoom;

	cairo_surface_t * rectBuffer = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
	cairo_t * crRect = cairo_create(rectBuffer);
	cairo_translate(crRect, -x, -y);
	cairo_scale(crRect, zoom, zoom);

	DocumentView view;
	view.limitArea(rect->x, rect->y, rect->width, rect->height);

	PdfCache * cache = this->view->xournal->getCache();
	PdfView::drawPage(cache, popplerPage, crRect, zoom, pageWidth, pageHeight);

	doc->lock();
	view.drawPage(this->view->page, crRect);
	doc->unlock();

	cairo_destroy(crRect);

	g_mutex_lock(this->view->drawingMutex);
	cairo_t * crPageBuffer = cairo_create(this->view->crBuffer);

	cairo_set_operator(crPageBuffer, CAIRO_OPERATOR_SOURCE);
	cairo_set_source_surface(crPageBuffer, rectBuffer, x, y);
	cairo_rectangle(crPageBuffer, x, y, width, height);
	cairo_fill(crPageBuffer);

	cairo_destroy(crPageBuffer);

	cairo_surface_destroy(rectBuffer);

	g_mutex_unlock(this->view->drawingMutex);
}

void RenderJob::run() {
	double zoom = this->view->xournal->getZoom();

	if (this->view->repaintComplete) {
		Document * doc = this->view->xournal->getDocument();
		GtkAllocation alloc;
		gtk_widget_get_allocation(this->view->widget, &alloc);

		cairo_surface_t * crBuffer = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, alloc.width, alloc.height);
		cairo_t * cr2 = cairo_create(crBuffer);
		cairo_scale(cr2, zoom, zoom);

		XojPopplerPage * popplerPage = NULL;

		doc->lock();

		if (this->view->page->getBackgroundType() == BACKGROUND_TYPE_PDF) {
			int pgNo = this->view->page->getPdfPageNr();
			popplerPage = doc->getPdfPage(pgNo);
		}

		DocumentView view;
		PdfView::drawPage(this->view->xournal->getCache(), popplerPage, cr2, zoom, this->view->page->getWidth(), this->view->page->getHeight());
		view.drawPage(this->view->page, cr2);
		cairo_destroy(cr2);
		g_mutex_lock(this->view->drawingMutex);

		if (this->view->crBuffer) {
			cairo_surface_destroy(this->view->crBuffer);
		}
		this->view->crBuffer = crBuffer;

		g_mutex_unlock(this->view->drawingMutex);
		doc->unlock();

		gdk_threads_enter();
		gtk_widget_queue_draw(this->view->widget);
		gdk_threads_leave();

	} else {
		for (GList * l = this->view->repaintRect; l != NULL; l = l->next) {
			Rectangle * rect = (Rectangle *) l->data;
			repaintRectangle(rect, zoom);
			gdk_threads_enter();
			gtk_widget_queue_draw_area(this->view->widget, rect->x * zoom, rect->y * zoom, rect->width * zoom, rect->height * zoom);
			gdk_threads_leave();
		}
	}

	g_mutex_lock(this->view->repaintRectMutex);

	// delete all rectangles
	this->view->repaintComplete = false;
	for (GList * l = this->view->repaintRect; l != NULL; l = l->next) {
		Rectangle * rect = (Rectangle *) l->data;
		delete rect;
	}
	g_list_free(this->view->repaintRect);
	this->view->repaintRect = NULL;

	g_mutex_unlock(this->view->repaintRectMutex);
}

JobType RenderJob::getType() {
	return JOB_TYPE_RENDER;
}
