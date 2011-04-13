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

class RepaintWidgetHandler {
public:
	RepaintWidgetHandler(GtkWidget * width) {
		this->mutex = g_mutex_new();
		this->widget = width;
		this->complete = false;
		this->rects = NULL;
		this->rescaleId = 0;
	}

public:
	void repaintComplete() {
		g_mutex_lock(this->mutex);
		this->complete = true;
		for(GList * l = this->rects; l != NULL; l = l->next) {
			delete (Rectangle *)l->data;
		}
		g_list_free(this->rects);
		this->rects = NULL;

		addRepaintCallback();

		g_mutex_unlock(this->mutex);
	}

	void repaintRects(Rectangle * rect) {
		g_mutex_lock(this->mutex);
		if(this->complete) {
			delete rect;
		} else {
			this->rects = g_list_prepend(this->rects, rect);
		}
		addRepaintCallback();

		g_mutex_unlock(this->mutex);
	}

private:
	static bool idleRepaint(RepaintWidgetHandler * data) {
		g_mutex_lock(data->mutex);
		bool complete = data->complete;
		GList * rects = data->rects;
		GtkWidget * widget = data->widget;

		data->rects = NULL;
		data->complete = false;
		data->rescaleId = 0;

		g_mutex_unlock(data->mutex);

		gdk_threads_enter();

		gtk_widget_queue_draw(data->widget);

		if(complete) {
//			gtk_widget_queue_draw(data->widget);
		} else {
			for (GList * l = rects; l != NULL; l = l->next) {
				Rectangle * rect = (Rectangle *) l->data;
//				gtk_widget_queue_draw_area(widget, rect->x, rect->y, rect->width, rect->height);
				delete rect;
			}
			g_list_free(rects);
		}

		gdk_flush();

		gdk_threads_leave();

		// do not call again
		return false;
	}

	void addRepaintCallback() {
		if(this->rescaleId) {
			return;
		}

		this->rescaleId = g_idle_add((GSourceFunc)idleRepaint, this);
	}

private:
	GMutex * mutex;

	int rescaleId;

	bool complete;
	GList * rects;
	GtkWidget * widget;
};

RepaintWidgetHandler * handler = NULL;

void RenderJob::run() {
	XOJ_CHECK_TYPE(RenderJob);

	if(handler == NULL) {
		handler = new RepaintWidgetHandler(this->view->getXournal()->getWidget());
	}

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

		handler->repaintComplete();
	} else {
		for (GList * l = this->view->repaintRects; l != NULL; l = l->next) {
			Rectangle * rect = (Rectangle *) l->data;
			rerenderRectangle(rect);

			rect = this->view->rectOnWidget(rect->x, rect->y, rect->width, rect->height);
			handler->repaintRects(rect);
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
}

JobType RenderJob::getType() {
	XOJ_CHECK_TYPE(RenderJob);

	return JOB_TYPE_RENDER;
}
