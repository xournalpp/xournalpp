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
{
	XOJ_INIT_TYPE(PreviewJob);

	this->sidebarPreview = sidebar;
	this->crBuffer = NULL;
	this->cr2 = NULL;
	this->zoom = 0;
}

PreviewJob::~PreviewJob()
{
	XOJ_CHECK_TYPE(PreviewJob);

	this->sidebarPreview = NULL;

	XOJ_RELEASE_TYPE(PreviewJob);
}

void* PreviewJob::getSource()
{
	XOJ_CHECK_TYPE(PreviewJob);

	return this->sidebarPreview;
}

JobType PreviewJob::getType()
{
	XOJ_CHECK_TYPE(PreviewJob);

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

	gdk_threads_enter();
	gtk_widget_queue_draw(this->sidebarPreview->widget);
	gdk_threads_leave();

	g_mutex_unlock(&this->sidebarPreview->drawingMutex);
}

void PreviewJob::drawBackgroundPdf(Document* doc)
{
	int pgNo = this->sidebarPreview->page->getPdfPageNr();
	XojPopplerPage* popplerPage = doc->getPdfPage(pgNo);
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

	cairo_matrix_t defaultMatrix = {0};
	cairo_get_matrix(cr2, &defaultMatrix);
	cairo_set_matrix(cr2, &defaultMatrix);

	cairo_set_operator(cr2, CAIRO_OPERATOR_SOURCE);

	cairo_set_source_rgb(cr2, 1, 1, 1);
	cairo_rectangle(cr2, 0, 0, Shadow::getShadowTopLeftSize() + 2, cairo_image_surface_get_width(crBuffer));
	cairo_rectangle(cr2, 0, 0, cairo_image_surface_get_height(crBuffer), Shadow::getShadowTopLeftSize() + 2);

	cairo_rectangle(cr2, cairo_image_surface_get_width(crBuffer) - Shadow::getShadowBottomRightSize() - 2, 0,
					Shadow::getShadowBottomRightSize() + 2, cairo_image_surface_get_height(crBuffer));
	cairo_rectangle(cr2, 0, cairo_image_surface_get_height(crBuffer)- Shadow::getShadowBottomRightSize() - 2,
			cairo_image_surface_get_width(crBuffer), Shadow::getShadowBottomRightSize() + 2);

	cairo_fill(cr2);

	cairo_destroy(cr2);
}

void PreviewJob::run()
{
	XOJ_CHECK_TYPE(PreviewJob);

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

	if (this->sidebarPreview->page->getBackgroundType() == BACKGROUND_TYPE_PDF)
	{
		drawBackgroundPdf(doc);
	}

	drawPage(layer);

	doc->unlock();

	finishPaint();
}
