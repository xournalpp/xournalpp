#include "PreviewJob.h"

#include <memory>  // for __s...
#include <mutex>   // for mutex
#include <vector>  // for vector

#include <glib-object.h>  // for g_o...
#include <gtk/gtk.h>      // for Gtk...

#include "control/Control.h"                                      // for Con...
#include "control/jobs/Job.h"                                     // for JOB...
#include "gui/Shadow.h"                                           // for Shadow
#include "gui/sidebar/previews/base/SidebarPreviewBase.h"         // for Sid...
#include "gui/sidebar/previews/base/SidebarPreviewBaseEntry.h"    // for Sid...
#include "gui/sidebar/previews/layer/SidebarPreviewLayerEntry.h"  // for Sid...
#include "model/Document.h"                                       // for Doc...
#include "model/Layer.h"                                          // for Layer
#include "model/PageRef.h"                                        // for Pag...
#include "model/XojPage.h"                                        // for Xoj...
#include "util/Util.h"                                            // for exe...
#include "view/DocumentView.h"                                    // for Doc...
#include "view/LayerView.h"                                       // for Lay...
#include "view/View.h"                                            // for Con...
#include "view/background/BackgroundView.h"                       // for BAC...

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

void PreviewJob::drawPage() {
    ConstPageRef page = this->sidebarPreview->page;
    Document* doc = this->sidebarPreview->sidebar->getControl()->getDocument();
    DocumentView view;
    view.setPdfCache(this->sidebarPreview->sidebar->getCache());
    PreviewRenderType type = this->sidebarPreview->getRenderType();
    Layer::Index layer = 0;

    doc->lock();

    // getLayer is not defined for page preview
    if (type != RENDER_TYPE_PAGE_PREVIEW) {
        layer = (dynamic_cast<SidebarPreviewLayerEntry*>(this->sidebarPreview))->getLayer();
    }

    auto context = xoj::view::Context::createDefault(cr2);

    switch (type) {
        case RENDER_TYPE_PAGE_PREVIEW:
            // render all layers
            view.drawPage(page, cr2, true);
            break;

        case RENDER_TYPE_PAGE_LAYER:
            // render single layer
            view.initDrawing(page, cr2, true);
            if (layer == 0) {
                view.drawBackground(xoj::view::BACKGROUND_SHOW_ALL);
            } else {
                const Layer* drawLayer = page->getLayers()[layer - 1];
                xoj::view::LayerView layerView(drawLayer);
                layerView.draw(context);
            }
            view.finializeDrawing();
            break;

        case RENDER_TYPE_PAGE_LAYERSTACK: {
            // render all layers up to layer
            view.initDrawing(page, cr2, true);
            view.drawBackground(xoj::view::BACKGROUND_SHOW_ALL);
            const auto& layers = page->getLayers();
            for (Layer::Index i = 0; i < layer; i++) {
                const Layer* drawLayer = layers[i];
                xoj::view::LayerView layerView(drawLayer);
                layerView.draw(context);
            }
            view.finializeDrawing();
            break;
        }
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
