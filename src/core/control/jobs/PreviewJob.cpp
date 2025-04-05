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
#include "view/background/BackgroundFlags.h"                      // for BAC...

PreviewJob::PreviewJob(SidebarPreviewBaseEntry* sidebar): sidebarPreview(sidebar) {}

PreviewJob::~PreviewJob() { this->sidebarPreview = nullptr; }

void PreviewJob::onDelete() { this->sidebarPreview = nullptr; }

auto PreviewJob::getSource() -> void* { return this->sidebarPreview; }

auto PreviewJob::getType() -> JobType { return JOB_TYPE_PREVIEW; }

void PreviewJob::initGraphics() {
    auto w = this->sidebarPreview->imageWidth;
    auto h = this->sidebarPreview->imageHeight;
    auto DPIscaling = this->sidebarPreview->DPIscaling;
    buffer.reset(cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w * DPIscaling, h * DPIscaling), xoj::util::adopt);
    cairo_surface_set_device_scale(buffer.get(), DPIscaling, DPIscaling);
    cr.reset(cairo_create(buffer.get()), xoj::util::adopt);
    double zoom = this->sidebarPreview->sidebar->getZoom();
    cairo_translate(cr.get(), Shadow::getShadowTopLeftSize() + 2, Shadow::getShadowTopLeftSize() + 2);
    cairo_scale(cr.get(), zoom, zoom);
}

void PreviewJob::finishPaint() {
    auto lock = std::lock_guard(this->sidebarPreview->drawingMutex);
    this->sidebarPreview->buffer = std::move(this->buffer);
    Util::execInUiThread([btn = this->sidebarPreview->button]() { gtk_widget_queue_draw(btn.get()); });
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

    auto context = xoj::view::Context::createDefault(cr.get());

    switch (type) {
        case RENDER_TYPE_PAGE_PREVIEW:
            // render all layers
            view.drawPage(page, cr.get(), true);
            break;

        case RENDER_TYPE_PAGE_LAYER:
            // render single layer
            view.initDrawing(page, cr.get(), true);
            if (layer == 0) {
                auto flags = xoj::view::BACKGROUND_SHOW_ALL;
                flags.forceVisible = xoj::view::FORCE_VISIBLE;
                view.drawBackground(flags);
            } else {
                view.drawBackground(xoj::view::BACKGROUND_FORCE_PAINT_BACKGROUND_COLOR_ONLY);
                const Layer* drawLayer = page->getLayersView()[layer - 1];
                xoj::view::LayerView layerView(drawLayer);
                layerView.draw(context);
            }
            view.finializeDrawing();
            break;

        case RENDER_TYPE_PAGE_LAYERSTACK: {
            // render all layers up to layer
            view.initDrawing(page, cr.get(), true);
            auto flags = xoj::view::BACKGROUND_SHOW_ALL;
            flags.forceVisible = xoj::view::FORCE_VISIBLE;
            view.drawBackground(flags);
            const auto layers = page->getLayersView();
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

    doc->unlock();
}

void PreviewJob::clipToPage() {
    // Only render within the preview page. Without this, the when preview jobs attempt
    // to clear the display, we fill a region larger than the inside of the preview page!
    cairo_rectangle(cr.get(), 0, 0, this->sidebarPreview->page->getWidth(), this->sidebarPreview->page->getHeight());
    cairo_clip(cr.get());
}

void PreviewJob::run() {
    if (this->sidebarPreview == nullptr) {
        return;
    }

    initGraphics();
    clipToPage();
    drawPage();
    finishPaint();
}
