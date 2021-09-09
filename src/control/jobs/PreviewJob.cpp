#include "PreviewJob.h"

#include "control/Control.h"
#include "gui/Shadow.h"
#include "gui/sidebar/previews/base/SidebarPreviewBase.h"
#include "gui/sidebar/previews/base/SidebarPreviewBaseEntry.h"
#include "gui/sidebar/previews/layer/SidebarPreviewLayerEntry.h"
#include "model/Document.h"
#include "view/DocumentView.h"
#include "view/PdfView.h"

PreviewJob::PreviewJob(SidebarPreviewBaseEntry* sidebar): sidebarPreview(sidebar) {}

PreviewJob::~PreviewJob() { this->sidebarPreview = nullptr; }

void PreviewJob::onDelete() { this->sidebarPreview = nullptr; }

auto PreviewJob::getSource() -> void* { return this->sidebarPreview; }

auto PreviewJob::getType() -> JobType { return JOB_TYPE_PREVIEW; }

void PreviewJob::initGraphics() {
    GtkAllocation alloc;
    gtk_widget_get_allocation(this->sidebarPreview->widget, &alloc);
    crBuffer = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, alloc.width, alloc.height);
    zoom = this->sidebarPreview->sidebar->getZoom();
    cr2 = cairo_create(crBuffer);
}

void PreviewJob::drawBorder() {
    cairo_translate(cr2, Shadow::getShadowTopLeftSize() + 2, Shadow::getShadowTopLeftSize() + 2);
    cairo_scale(cr2, zoom, zoom);
}

void PreviewJob::finishPaint() {
    this->sidebarPreview->drawingMutex.lock();

    if (this->sidebarPreview->crBuffer) {
        cairo_surface_destroy(this->sidebarPreview->crBuffer);
    }
    this->sidebarPreview->crBuffer = crBuffer;

    // The preview widget can be referenced after this is deleted.
    // Only it should be referenced in the callback.
    GtkWidget* previewWidget = this->sidebarPreview->widget;
    g_object_ref(previewWidget);

    Util::execInUiThread([previewWidget]() {
        gtk_widget_queue_draw(previewWidget);
        g_object_unref(previewWidget);
    });

    this->sidebarPreview->drawingMutex.unlock();
}

void PreviewJob::drawBackgroundPdf(Document* doc) {
    int pgNo = this->sidebarPreview->page->getPdfPageNr();
    XojPdfPageSPtr popplerPage = doc->getPdfPage(pgNo);

    PdfView::drawPage(this->sidebarPreview->sidebar->getCache(), popplerPage, cr2, zoom,
                      this->sidebarPreview->page->getWidth(), this->sidebarPreview->page->getHeight());
}

void PreviewJob::drawPage() {
    DocumentView view;
    PageRef page = this->sidebarPreview->page;
    Document* doc = this->sidebarPreview->sidebar->getControl()->getDocument();
    PreviewRenderType type = this->sidebarPreview->getRenderType();
    int layer;

    doc->lock();

    // getLayer is not defined for page preview
    if (type != RENDER_TYPE_PAGE_PREVIEW) {
        layer = (dynamic_cast<SidebarPreviewLayerEntry*>(this->sidebarPreview))->getLayer();
    }

    // Pdf::drawPage needs to go before DocumentView::initDrawing until DocumentView learns to do it and the first
    // switch block can go away and the layer assignment into the remaining switch block.
    switch (type) {
        case RENDER_TYPE_PAGE_LAYER:
            if (layer != -1) {
                break;  // out
            }
            [[fallthrough]];

        case RENDER_TYPE_PAGE_LAYERSTACK:
        case RENDER_TYPE_PAGE_PREVIEW:
            if (page->getBackgroundType().isPdfPage()) {
                drawBackgroundPdf(doc);
            }
            break;

        default:
            // unknown type
            break;
    }

    switch (type) {
        case RENDER_TYPE_PAGE_PREVIEW:
            // render all layers
            view.drawPage(page, cr2, true);
            break;

        case RENDER_TYPE_PAGE_LAYER:
            // render single layer
            view.initDrawing(page, cr2, true);
            if (layer == -1) {
                view.drawBackground();
            } else {
                Layer* drawLayer = (*page->getLayers())[layer];
                view.drawLayer(cr2, drawLayer);
            }
            view.finializeDrawing();
            break;

        case RENDER_TYPE_PAGE_LAYERSTACK:
            // render all layers up to layer
            view.initDrawing(page, cr2, true);
            view.drawBackground();
            for (int i = 0; i <= layer; i++) {
                Layer* drawLayer = (*page->getLayers())[i];
                view.drawLayer(cr2, drawLayer);
            }
            view.finializeDrawing();
            break;

        default:
            // unknown type
            break;
    }

    cairo_destroy(cr2);
    doc->unlock();
}

void PreviewJob::clipToPage() {
    // Only render within the preview page. Without this, the when preview jobs attempt
    // to clear the display, we fill a region larger than the inside of the preview page!
    cairo_rectangle(cr2, 0, 0, this->sidebarPreview->page->getWidth(), this->sidebarPreview->page->getHeight());
    cairo_clip(cr2);
}

void PreviewJob::run() {
    if (this->sidebarPreview == nullptr) {
        return;
    }

    initGraphics();
    drawBorder();
    clipToPage();
    drawPage();
    finishPaint();
}
