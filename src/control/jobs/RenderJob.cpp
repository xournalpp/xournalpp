#include "RenderJob.h"
#include <gtk/gtk.h>
#include "../../gui/XournalView.h"
#include "../../view/PdfView.h"
#include "../../view/DocumentView.h"
#include "../../gui/PageView.h"
#include "../../model/Document.h"
#include "../../util/Rectangle.h"

RenderJob::RenderJob(PageView * view) {
	XOJ_INIT_TYPE(RenderJob);

	this->view = view;
	this->repaintComplete = false;
	this->repaintRect = NULL;
}

RenderJob::~RenderJob() {
	XOJ_CHECK_TYPE(RenderJob);

	this->view = NULL;

	XOJ_RELEASE_TYPE(RenderJob);
}

void * RenderJob::getSource() {
	XOJ_CHECK_TYPE(RenderJob);

	return this->view;
}

void RenderJob::repaintRectangle(RenderJob * renderJob, Rectangle * rect) {
	XOJ_CHECK_TYPE_OBJ(renderJob, RenderJob);

	PageView * view = renderJob->view;
	double zoom = view->xournal->getZoom();
	Document * doc = view->xournal->getDocument();

	doc->lock();

	double pageWidth = view->page.getWidth();
	double pageHeight = view->page.getHeight();

	doc->unlock();

	int x = rect->x * zoom;
	int y = rect->y * zoom;
	int width = rect->width * zoom;
	int height = rect->height * zoom;

	cairo_surface_t * rectBuffer = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
	cairo_t * crRect = cairo_create(rectBuffer);
	cairo_translate(crRect, -x, -y);
	cairo_scale(crRect, zoom, zoom);

	DocumentView v;
	v.limitArea(rect->x, rect->y, rect->width, rect->height);

	if (view->page.getBackgroundType() == BACKGROUND_TYPE_PDF) {
		int pgNo = view->page.getPdfPageNr();
		XojPopplerPage * popplerPage = doc->getPdfPage(pgNo);
		PdfCache * cache = view->xournal->getCache();
		PdfView::drawPage(cache, popplerPage, crRect, zoom, pageWidth, pageHeight);
	}

	doc->lock();
	v.drawPage(view->page, crRect, false);
	doc->unlock();

	cairo_destroy(crRect);

	g_mutex_lock(view->drawingMutex);
	cairo_t * crPageBuffer = cairo_create(view->crBuffer);

	cairo_set_operator(crPageBuffer, CAIRO_OPERATOR_SOURCE);
	cairo_set_source_surface(crPageBuffer, rectBuffer, x, y);
	cairo_rectangle(crPageBuffer, x, y, width, height);
	cairo_fill(crPageBuffer);

	cairo_destroy(crPageBuffer);

	cairo_surface_destroy(rectBuffer);

	g_mutex_unlock(view->drawingMutex);
}

void RenderJob::rerenderRectangle(Rectangle * rect) {
	XOJ_CHECK_TYPE(RenderJob);

	repaintRectangle(this, rect);
}

struct RepaintData {
	bool complete;
	GList * repaintRects;
	GtkWidget * widget;
};

bool idleRepaint(struct RepaintData * data) {
	gdk_threads_enter();

	if(data->complete) {
	} else {
		for (GList * l = data->repaintRects; l != NULL; l = l->next) {
			Rectangle * rect = (Rectangle *) l->data;
//			this->view->repaintRect(rect->x, rect->y, rect->width, rect->height);
			delete rect;
		}
		g_list_free(data->repaintRects);
	}


	// TODO: improve
	gtk_widget_queue_draw(data->widget);

	gdk_threads_leave();

	g_free(data);
	// do not call again
	return false;
}

void RenderJob::run() {
	XOJ_CHECK_TYPE(RenderJob);

	double zoom = this->view->xournal->getZoom();

	if (this->view->rerenderComplete) {
		Document * doc = this->view->xournal->getDocument();

		int dispWidth = this->view->getDisplayWidth();
		int dispHeight = this->view->getDisplayHeight();

		cairo_surface_t * crBuffer = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, dispWidth, dispHeight);
		cairo_t * cr2 = cairo_create(crBuffer);
		cairo_scale(cr2, zoom, zoom);

		XojPopplerPage * popplerPage = NULL;

		doc->lock();

		if (this->view->page.getBackgroundType() == BACKGROUND_TYPE_PDF) {
			int pgNo = this->view->page.getPdfPageNr();
			popplerPage = doc->getPdfPage(pgNo);
		}

		DocumentView view;
		int width = this->view->page.getWidth();
		int height = this->view->page.getHeight();

		PdfView::drawPage(this->view->xournal->getCache(), popplerPage, cr2, zoom, width, height);
		view.drawPage(this->view->page, cr2, false);

		cairo_destroy(cr2);
		g_mutex_lock(this->view->drawingMutex);

		if (this->view->crBuffer) {
			cairo_surface_destroy(this->view->crBuffer);
		}
		this->view->crBuffer = crBuffer;

		g_mutex_unlock(this->view->drawingMutex);
		doc->unlock();

		this->repaintComplete = true;
	} else {
		for (GList * l = this->view->repaintRects; l != NULL; l = l->next) {
			Rectangle * rect = (Rectangle *) l->data;
			rerenderRectangle(rect);

			this->repaintRect = g_list_append(this->repaintRect, new Rectangle(rect->x, rect->y, rect->width, rect->height));
		}
	}

	g_mutex_lock(this->view->repaintRectMutex);

	// delete all rectangles
	this->view->rerenderComplete = false;
	for (GList * l = this->view->repaintRects; l != NULL; l = l->next) {
		Rectangle * rect = (Rectangle *) l->data;
		delete rect;
	}
	g_list_free(this->view->repaintRects);
	this->view->repaintRects = NULL;

	g_mutex_unlock(this->view->repaintRectMutex);

	struct RepaintData * data = g_new(struct RepaintData, 1);
	data->complete = this->repaintComplete;
	data->repaintRects = this->repaintRect;
	data->widget = this->view->getXournal()->getWidget();

	if(this->repaintComplete && this->repaintRect) {
		g_warning("RenderJob::run:: repaint complete but repaintRect != NULL!");
	}

	g_idle_add((GSourceFunc)idleRepaint, data);

	// PORTABILITY: this is not working on Windows...
	//	gdk_threads_enter();
	//
	//	if (this->repaintComplete) {
	//		this->view->repaintPage();
	//	} else {
	//		for (GList * l = this->repaintRect; l != NULL; l = l->next) {
	//			Rectangle * rect = (Rectangle *) l->data;
	//			this->view->repaintRect(rect->x, rect->y, rect->width, rect->height);
	//			delete rect;
	//		}
	//		g_list_free(this->repaintRect);
	//		this->repaintRect = NULL;
	//	}
	//
	//	gdk_threads_leave();

}

JobType RenderJob::getType() {
	XOJ_CHECK_TYPE(RenderJob);

	return JOB_TYPE_RENDER;
}
