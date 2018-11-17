#include "RenderJob.h"
#include "RepaintWidgetHandler.h"

#include "gui/PageView.h"
#include "gui/XournalView.h"
#include "model/Document.h"
#include "view/DocumentView.h"
#include "view/PdfView.h"

#include <Rectangle.h>

#include <list>

RenderJob::RenderJob(XojPageView* view)
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

	XojPageView* view = renderJob->view;
	double zoom = view->xournal->getZoom();
	Document* doc = view->xournal->getDocument();

	if (!noThreads)
	{
		doc->lock();
	}

	double pageWidth = view->page->getWidth();
	double pageHeight = view->page->getHeight();

	if (!noThreads)
	{
		doc->unlock();
	}

	int x = rect->x * zoom;
	int y = rect->y * zoom;
	int width = rect->width * zoom;
	int height = rect->height * zoom;

	cairo_surface_t* rectBuffer = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
	cairo_t* crRect = cairo_create(rectBuffer);
	cairo_translate(crRect, -x, -y);
	cairo_scale(crRect, zoom, zoom);

	DocumentView v;
	v.limitArea(rect->x, rect->y, rect->width, rect->height);

	if (view->page->getBackgroundType() == BACKGROUND_TYPE_PDF)
	{
		int pgNo = view->page->getPdfPageNr();
		XojPopplerPage* popplerPage = doc->getPdfPage(pgNo);
		PdfCache* cache = view->xournal->getCache();
		PdfView::drawPage(cache, popplerPage, crRect, zoom, pageWidth, pageHeight);
	}

	if (!noThreads)
	{
		doc->lock();
	}

	v.drawPage(view->page, crRect, false);

	if (!noThreads)
	{
		doc->unlock();
	}

	cairo_destroy(crRect);

	if (!noThreads)
	{
		g_mutex_lock(&view->drawingMutex);
	}

	cairo_t * crPageBuffer = cairo_create(view->crBuffer);

	cairo_set_operator(crPageBuffer, CAIRO_OPERATOR_SOURCE);
	cairo_set_source_surface(crPageBuffer, rectBuffer, x, y);
	cairo_rectangle(crPageBuffer, x, y, width, height);
	cairo_fill(crPageBuffer);

	cairo_destroy(crPageBuffer);

	cairo_surface_destroy(rectBuffer);

	if (!noThreads)
	{
		g_mutex_unlock(&view->drawingMutex);
	}
}

void RenderJob::rerenderRectangle(Rectangle* rect, bool noThreads)
{
	XOJ_CHECK_TYPE(RenderJob);

	rerenderRectangle(this, rect, noThreads);
}

RepaintWidgetHandler* handler = NULL;

void RenderJob::run(bool noThreads)
{
	XOJ_CHECK_TYPE(RenderJob);

	if (handler == NULL)
	{
		handler = new RepaintWidgetHandler(this->view->getXournal()->getWidget());
	}

	double zoom = this->view->xournal->getZoom();

	g_mutex_lock(&this->view->repaintRectMutex);

	bool rerenderComplete = this->view->rerenderComplete;
	std::vector<Rectangle*> rerenderRects = this->view->rerenderRects;
	this->view->rerenderRects.clear();

	this->view->rerenderComplete = false;

	g_mutex_unlock(&this->view->repaintRectMutex);


	if (rerenderComplete)
	{
		Document* doc = this->view->xournal->getDocument();

		int dispWidth = this->view->getDisplayWidth();
		int dispHeight = this->view->getDisplayHeight();

		cairo_surface_t* crBuffer = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, dispWidth, dispHeight);
		cairo_t* cr2 = cairo_create(crBuffer);
		cairo_scale(cr2, zoom, zoom);

		XojPopplerPage* popplerPage = NULL;

		if (!noThreads)
		{
			doc->lock();
		}

		if (this->view->page->getBackgroundType() == BACKGROUND_TYPE_PDF)
		{
			int pgNo = this->view->page->getPdfPageNr();
			popplerPage = doc->getPdfPage(pgNo);
		}

		DocumentView view;
		int width = this->view->page->getWidth();
		int height = this->view->page->getHeight();

		PdfView::drawPage(this->view->xournal->getCache(), popplerPage, cr2, zoom, width, height);
		view.drawPage(this->view->page, cr2, false);

		cairo_destroy(cr2);
		if (!noThreads)
		{
			g_mutex_lock(&this->view->drawingMutex);
		}

		if (this->view->crBuffer)
		{
			cairo_surface_destroy(this->view->crBuffer);
		}
		this->view->crBuffer = crBuffer;

		if (!noThreads)
		{
			g_mutex_unlock(&this->view->drawingMutex);
			doc->unlock();
		}

		handler->repaintComplete();
	}
	else
	{
		for (Rectangle* rect : rerenderRects)
		{
			rerenderRectangle(rect, noThreads);

			rect = this->view->rectOnWidget(rect->x, rect->y, rect->width, rect->height);
			handler->repaintRects(rect);
		}
	}


	// delete all rectangles
	for (Rectangle* rect : rerenderRects)
	{
		delete rect;
		rect = NULL;
	}
}

JobType RenderJob::getType()
{
	XOJ_CHECK_TYPE(RenderJob);

	return JOB_TYPE_RENDER;
}
