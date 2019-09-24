#include "PreviewJob.h"

#include "control/Control.h"
#include "gui/Shadow.h"
#include "gui/sidebar/previews/base/SidebarPreviewBaseEntry.h"
#include "gui/sidebar/previews/base/SidebarPreviewBase.h"
#include "gui/sidebar/previews/layer/SidebarPreviewLayerEntry.h"
#include "model/Document.h"
#include "view/PdfView.h"
#include "view/DocumentView.h"

PreviewJob::PreviewJob(SidebarPreviewBaseEntry* sidebar)
 : sidebarPreview(sidebar)
{
}

PreviewJob::~PreviewJob()
{
	this->sidebarPreview = nullptr;
}

void* PreviewJob::getSource()
{
	return this->sidebarPreview;
}

JobType PreviewJob::getType()
{
	return JOB_TYPE_PREVIEW;
}

void PreviewJob::initGraphics()
{
	GtkAllocation alloc;
	gtk_widget_get_allocation(this->sidebarPreview->widget, &alloc);
	crBuffer = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, alloc.width, alloc.height);
	zoom = this->sidebarPreview->sidebar->getZoom();
	cr2 = cairo_create(crBuffer);
}

void PreviewJob::drawBorder()
{
	cairo_translate(cr2, Shadow::getShadowTopLeftSize() + 2, Shadow::getShadowTopLeftSize() + 2);
	cairo_scale(cr2, zoom, zoom);
}

void PreviewJob::finishPaint()
{
	g_mutex_lock(&this->sidebarPreview->drawingMutex);

	if (this->sidebarPreview->crBuffer)
	{
		cairo_surface_destroy(this->sidebarPreview->crBuffer);
	}
	this->sidebarPreview->crBuffer = crBuffer;

	// Make sure the Job does not get deleted until the
	// Repaint is also finished in UI Thread
	ref();

	Util::execInUiThread([=]() {
		gtk_widget_queue_draw(this->sidebarPreview->widget);

		// After the UI job is also done, it can be unreferenced
		unref();
	});

	g_mutex_unlock(&this->sidebarPreview->drawingMutex);
}

void PreviewJob::drawBackgroundPdf(Document* doc)
{
	int pgNo = this->sidebarPreview->page->getPdfPageNr();
	XojPdfPageSPtr popplerPage = doc->getPdfPage(pgNo);
	PdfView::drawPage(this->sidebarPreview->sidebar->getCache(), popplerPage, cr2, zoom,
					  this->sidebarPreview->page->getWidth(), this->sidebarPreview->page->getHeight());
}

void PreviewJob::drawPage(int layer)
{
	DocumentView view;
	PageRef page = this->sidebarPreview->page;

	if (layer == -100)
	{
		// render all layer
		view.drawPage(page, cr2, true);
	}
	else if (layer == -1)
	{
		// draw only background
		view.initDrawing(page, cr2, true);
		view.drawBackground();
		view.finializeDrawing();
	}
	else
	{
		view.initDrawing(page, cr2, true);

		Layer* drawLayer = (*page->getLayers())[layer];
		view.drawLayer(cr2, drawLayer);

		view.finializeDrawing();
	}

	cairo_destroy(cr2);
}

void PreviewJob::run()
{
	initGraphics();
	drawBorder();

	Document* doc = this->sidebarPreview->sidebar->getControl()->getDocument();
	doc->lock();

	PreviewRenderType type = this->sidebarPreview->getRenderType();
	int layer = -100; // all layer

	if (RENDER_TYPE_PAGE_LAYER == type)
	{
		layer = ((SidebarPreviewLayerEntry*)this->sidebarPreview)->getLayer();
	}

	if (this->sidebarPreview->page->getBackgroundType().isPdfPage())
	{
		drawBackgroundPdf(doc);
	}

	drawPage(layer);

	doc->unlock();

	finishPaint();
}
