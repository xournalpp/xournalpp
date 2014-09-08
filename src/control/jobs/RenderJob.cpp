#include "RenderJob.h"
#include <gtk/gtk.h>
#include "../../gui/XournalView.h"
#include "../../view/PdfView.h"
#include "../../view/DocumentView.h"
#include "../../gui/PageView.h"
#include "../../model/Document.h"
#include <Rectangle.h>

RenderJob::RenderJob(PageView* view)
{
	XOJ_INIT_TYPE(RenderJob);

	this->view = view;
}

RenderJob::~RenderJob()
{
	XOJ_CHECK_TYPE(RenderJob);

	this->view = NULL;

	XOJ_RELEASE_TYPE(RenderJob);
}

void* RenderJob::getSource()
{
	XOJ_CHECK_TYPE(RenderJob);

	return this->view;
}

void RenderJob::rerenderRectangle(RenderJob* renderJob, Rectangle* rect, bool noThreads)
{
	XOJ_CHECK_TYPE_OBJ(renderJob, RenderJob);

	PageView* view = renderJob->view;
	double zoom = view->getXournal()->getZoom();
	Document* doc = view->getXournal()->getDocument();
	PageRef page = view->getPage();
	GMutex* drawingMutex = view->getDrawingMutex();

	if(!noThreads)
		doc->lock();

	double pageWidth = page->getWidth();
	double pageHeight = page->getHeight();

	if(!noThreads)
		doc->unlock();

	int x = rect->x * zoom;
	int y = rect->y * zoom;
	int width = rect->width * zoom;
	int height = rect->height * zoom;

	cairo_surface_t* rectBuffer = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
	                                                         width, height);
	cairo_t* crRect = cairo_create(rectBuffer);
	cairo_translate(crRect, -x, -y);
	cairo_scale(crRect, zoom, zoom);

	DocumentView v;
	v.limitArea(rect->x, rect->y, rect->width, rect->height);

	if (page->getBackgroundType() == BACKGROUND_TYPE_PDF)
	{
		int pgNo = page->getPdfPageNr();
		XojPopplerPage* popplerPage = doc->getPdfPage(pgNo);
		PdfCache* cache = view->getXournal()->getCache();
		PdfView::drawPage(cache, popplerPage, crRect, zoom, pageWidth, pageHeight);
	}

	if(!noThreads)
		doc->lock();
	v.drawPage(page, crRect, false);
	if(!noThreads)
		doc->unlock();

	cairo_destroy(crRect);

	if(!noThreads)
		g_mutex_lock(drawingMutex);

	cairo_t * crPageBuffer = cairo_create(view->getViewBuffer());

	cairo_set_operator(crPageBuffer, CAIRO_OPERATOR_SOURCE);
	cairo_set_source_surface(crPageBuffer, rectBuffer, x, y);
	cairo_rectangle(crPageBuffer, x, y, width, height);
	cairo_fill(crPageBuffer);

	cairo_destroy(crPageBuffer);

	cairo_surface_destroy(rectBuffer);

	if(!noThreads)
		g_mutex_unlock(drawingMutex);
}

void RenderJob::rerenderRectangle(Rectangle* rect, bool noThreads)
{
	XOJ_CHECK_TYPE(RenderJob);

	rerenderRectangle(this, rect, noThreads);
}

class RepaintWidgetHandler
{
public:
	RepaintWidgetHandler(GtkWidget * width) {
		g_mutex_init(&this->mutex);
		this->widget = width;
		this->complete = false;
		this->rects = NULL;
		this->rescaleId = 0;
	}

public:
	void repaintComplete() {
		g_mutex_lock(&this->mutex);
		this->complete = true;
		for(GList* l = this->rects; l != NULL; l = l->next)
		{
			delete (Rectangle*)l->data;
		}
		g_list_free(this->rects);
		this->rects = NULL;

		addRepaintCallback();

		g_mutex_unlock(&this->mutex);
	}

	void repaintRects(Rectangle * rect) {
		g_mutex_lock(&this->mutex);
		if(this->complete) {
			delete rect;
		}
		else
		{
			this->rects = g_list_prepend(this->rects, rect);
		}
		addRepaintCallback();

		g_mutex_unlock(&this->mutex);
	}

private:
	static bool idleRepaint(RepaintWidgetHandler * data) {
		g_mutex_lock(&data->mutex);
		bool complete = data->complete;
		GList* rects = data->rects;

		data->rects = NULL;
		data->complete = false;
		data->rescaleId = 0;

		g_mutex_unlock(&data->mutex);

		gdk_threads_enter();

		gtk_widget_queue_draw(data->widget);

		if(complete)
		{
			//			gtk_widget_queue_draw(data->widget);
		}
		else
		{
			for (GList* l = rects; l != NULL; l = l->next)
			{
				Rectangle* rect = (Rectangle*) l->data;
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

	void addRepaintCallback()
	{
		if(this->rescaleId)
		{
			return;
		}

		this->rescaleId = g_idle_add((GSourceFunc)idleRepaint, this);
	}

private:
	GMutex mutex;

	int rescaleId;

	bool complete;
	GList* rects;
	GtkWidget* widget;
};

RepaintWidgetHandler* handler = NULL;

void RenderJob::run(bool noThreads)
{
	XOJ_CHECK_TYPE(RenderJob);


	if(handler == NULL)
	{
		handler = new RepaintWidgetHandler(this->view->getXournal()->getWidget());
	}

	double zoom = this->view->getXournal()->getZoom();
	GMutex* drawingMutex = this->view->getDrawingMutex();
	GMutex* repaintMutex = this->view->getRepaintMutex();
	PageRef page = this->view->getPage();

	g_mutex_lock(repaintMutex);

	bool rerenderComplete = this->view->rerenderComplete;
	GList* rerenderRects = this->view->rerenderRects;
	this->view->rerenderRects = NULL;

	this->view->rerenderComplete = false;

	g_mutex_unlock(repaintMutex);


	if (rerenderComplete)
	{
		Document* doc = this->view->getXournal()->getDocument();

		int dispWidth = this->view->getDisplayWidth();
		int dispHeight = this->view->getDisplayHeight();

		cairo_surface_t* crBuffer = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
		                                                       dispWidth, dispHeight);

		cairo_t* cr = cairo_create(crBuffer);
		cairo_scale(cr, zoom, zoom);

		XojPopplerPage* popplerPage = NULL;

		if(!noThreads)
			doc->lock();

		if (this->view->getPage()->getBackgroundType() == BACKGROUND_TYPE_PDF)
		{
			int pgNo = page->getPdfPageNr();
			popplerPage = doc->getPdfPage(pgNo);
		}

		DocumentView view;
		int width = page->getWidth();
		int height = page->getHeight();

		PdfView::drawPage(this->view->getXournal()->getCache(), popplerPage, cr, zoom,
		                  width, height);
		view.drawPage(page, cr, false);

		cairo_destroy(cr);
		if(!noThreads)
			g_mutex_lock(drawingMutex);

		this->view->deleteViewBuffer();
		this->view->setViewBuffer(crBuffer);

		if(!noThreads)
		{
			g_mutex_unlock(drawingMutex);
			doc->unlock();
		}

		handler->repaintComplete();
	}
	else
	{
		for (GList* l = rerenderRects; l != NULL; l = l->next)
		{
			Rectangle* rect = (Rectangle*) l->data;
			rerenderRectangle(rect, noThreads);

			rect = this->view->rectOnWidget(rect->x, rect->y, rect->width, rect->height);
			handler->repaintRects(rect);
		}
	}


	// delete all rectangles
	for (GList* l = rerenderRects; l != NULL; l = l->next)
	{
		Rectangle* rect = (Rectangle*) l->data;
		delete rect;
	}
	g_list_free(rerenderRects);
}

JobType RenderJob::getType()
{
	XOJ_CHECK_TYPE(RenderJob);

	return JOB_TYPE_RENDER;
}
