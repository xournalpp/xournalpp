#include "PreviewJob.h"
#include "../../gui/sidebar/previews/SidebarPreviews.h"
#include "../../gui/sidebar/previews/SidebarPreviewPage.h"
#include "../../gui/Shadow.h"
#include "../../view/PdfView.h"
#include "../../view/DocumentView.h"
#include "../../model/Document.h"
#include "../../control/Control.h"

PreviewJob::PreviewJob(SidebarPreviewPage * sidebar) {
	XOJ_INIT_TYPE(PreviewJob);

	this->sidebarPreview = sidebar;
}

PreviewJob::~PreviewJob() {
	XOJ_CHECK_TYPE(PreviewJob);

	this->sidebarPreview = NULL;

	XOJ_RELEASE_TYPE(PreviewJob);
}

void * PreviewJob::getSource() {
	XOJ_CHECK_TYPE(PreviewJob);

	return this->sidebarPreview;
}

JobType PreviewJob::getType() {
	XOJ_CHECK_TYPE(PreviewJob);

	return JOB_TYPE_PREVIEW;
}

void PreviewJob::run() {
	XOJ_CHECK_TYPE(PreviewJob);

	GtkAllocation alloc;
	gtk_widget_get_allocation(this->sidebarPreview->widget, &alloc);

	cairo_surface_t * crBuffer = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, alloc.width, alloc.height);

	double zoom = this->sidebarPreview->sidebar->getZoom();

	cairo_t * cr2 = cairo_create(crBuffer);
	cairo_matrix_t defaultMatrix = { 0 };
	cairo_get_matrix(cr2, &defaultMatrix);

	cairo_translate(cr2, Shadow::getShadowTopLeftSize() + 2, Shadow::getShadowTopLeftSize() + 2);

	cairo_scale(cr2, zoom, zoom);

	Document * doc = this->sidebarPreview->sidebar->getControl()->getDocument();
	doc->lock();

	if (this->sidebarPreview->page.getBackgroundType() == BACKGROUND_TYPE_PDF) {
		int pgNo = this->sidebarPreview->page.getPdfPageNr();
		XojPopplerPage * popplerPage = doc->getPdfPage(pgNo);
		PdfView::drawPage(this->sidebarPreview->sidebar->getCache(), popplerPage, cr2, zoom, this->sidebarPreview->page.getWidth(), this->sidebarPreview->page.getHeight());
	}

	DocumentView view;
	view.drawPage(this->sidebarPreview->page, cr2, true);

	cairo_set_matrix(cr2, &defaultMatrix);

	cairo_set_operator(cr2, CAIRO_OPERATOR_SOURCE);

	cairo_set_source_rgb(cr2, 1, 1, 1);
	cairo_rectangle(cr2, 0, 0, Shadow::getShadowTopLeftSize() + 2, alloc.height);
	cairo_rectangle(cr2, 0, 0, alloc.height, Shadow::getShadowTopLeftSize() + 2);

	cairo_rectangle(cr2, alloc.width - Shadow::getShadowBottomRightSize() - 2, 0, Shadow::getShadowBottomRightSize() + 2, alloc.height);
	cairo_rectangle(cr2, 0, alloc.height - Shadow::getShadowBottomRightSize() - 2, alloc.width, Shadow::getShadowBottomRightSize() + 2);

	cairo_fill(cr2);

	cairo_destroy(cr2);


	doc->unlock();

	g_mutex_lock(this->sidebarPreview->drawingMutex);

	if (this->sidebarPreview->crBuffer) {
		cairo_surface_destroy(this->sidebarPreview->crBuffer);
	}
	this->sidebarPreview->crBuffer = crBuffer;

	gdk_threads_enter();
	gtk_widget_queue_draw(this->sidebarPreview->widget);
	gdk_threads_leave();

	g_mutex_unlock(this->sidebarPreview->drawingMutex);
}
