#include "RenderJob.h"

#include <cmath>
#include <future>

#include "control/Control.h"
#include "control/ToolHandler.h"
#include "gui/PageView.h"
#include "gui/XournalView.h"
#include "model/Document.h"
#include "util/Util.h"
#include "view/DocumentView.h"
#include "view/PdfView.h"

// Dimension of a tile in pixels
constexpr int tile_dim = 200;

RenderJob::RenderJob(XojPageView* view): view(view) {}

auto RenderJob::getSource() -> void* { return this->view; }

void RenderJob::rerenderTile(utl::Point<double> const& view_offset, utl::Point<double> const& document_offset) const {
    double zoom = view->xournal->getZoom();
    int dpiScaleFactor = this->view->xournal->getDpiScaleFactor();

    // Tile dimensions in view
    double view_offset_x = view_offset.x;
    double view_offset_y = view_offset.y;
    int view_width = tile_dim;
    int view_height = tile_dim;

    // Tile dimensions in document
    double document_offset_x = document_offset.x;
    double document_offset_y = document_offset.y;
    double document_width = tile_dim / zoom / static_cast<double>(dpiScaleFactor);
    double document_height = tile_dim / zoom / static_cast<double>(dpiScaleFactor);

    // Create cairo surface and context for tile
    cairo_surface_t* rectBuffer = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, view_width, view_height);
    cairo_t* crRect = cairo_create(rectBuffer);

    // Scale the tile for correct zoom and dpi
    cairo_scale(crRect, dpiScaleFactor * zoom, dpiScaleFactor * zoom);

    // Translate any drawings to the origin of the tile
    cairo_translate(crRect, -document_offset_x, -document_offset_y);

    // Create a view of the document for the area of this tile
    DocumentView v;
    Control* control = view->getXournal()->getControl();
    v.setMarkAudioStroke(control->getToolHandler()->getToolType() == TOOL_PLAY_OBJECT);
    v.limitArea(document_offset_x, document_offset_y, document_width, document_height);

    // Get the document
    Document* doc = view->xournal->getDocument();
    doc->lock();
    double pageWidth = view->page->getWidth();
    double pageHeight = view->page->getHeight();
    doc->unlock();

    // Draw the background layer
    bool backgroundVisible = view->page->isLayerVisible(0);
    if (backgroundVisible && view->page->getBackgroundType().isPdfPage()) {
        auto pgNo = view->page->getPdfPageNr();
        XojPdfPageSPtr popplerPage = doc->getPdfPage(pgNo);
        PdfCache* cache = view->xournal->getCache();
        PdfView::drawPage(cache, popplerPage, crRect, zoom * dpiScaleFactor, pageWidth, pageHeight);
    }

    // Draw the page
    doc->lock();
    v.drawPage(view->page, crRect, false);
    doc->unlock();

    /*
     * TODO: Remove once all done
     */
    /*cairo_set_source_rgb (crRect, 1, 0, 0);
    cairo_set_line_width(crRect, 1.0);
    cairo_rectangle(crRect, document_offset_x + 0.5, document_offset_y + 0.5, document_width - 1, document_height - 1);
    cairo_stroke(crRect);*/

    // Destroy the context
    cairo_destroy(crRect);

    /*
     * TODO: Remove once all done
     */
    /*cairo_t* cr_debug = cairo_create(rectBuffer);
    cairo_set_source_rgb (cr_debug, 0, 0, 0);
    cairo_set_line_width(cr_debug, 1.0);
    cairo_rectangle(cr_debug, 0.5, 0.5, view_width - 1, view_height - 1);
    cairo_stroke(cr_debug);
    cairo_destroy(cr_debug);*/

    // Create a context for drawing
    cairo_t* crPageBuffer = cairo_create(view->crBuffer);

    // Draw the tile
    cairo_set_operator(crPageBuffer, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_surface(crPageBuffer, rectBuffer, view_offset_x, view_offset_y);
    cairo_rectangle(crPageBuffer, view_offset_x, view_offset_y, view_width, view_height);
    cairo_fill(crPageBuffer);

    // Destroy the context
    cairo_destroy(crPageBuffer);

    // Destroy the surface of the tile
    cairo_surface_destroy(rectBuffer);
}

// TODO: Document that rectangle coordinates are in doc. coords (no zoom, no DPI scale)
void RenderJob::rerenderRectangle(Rectangle<double> const& rect) const {
    // Determine the amount of tiles we need for this rectangle in the view (take zoom and DPI into account)
    double zoom = view->xournal->getZoom();
    int dpiScaleFactor = this->view->xournal->getDpiScaleFactor();
    auto xNumTiles = static_cast<long>(std::ceil(rect.width * dpiScaleFactor * zoom / tile_dim));
    auto yNumTiles = static_cast<long>(std::ceil(rect.height * dpiScaleFactor * zoom / tile_dim));

    std::vector<std::future<void>> tileFutures;
    tileFutures.reserve(static_cast<unsigned long>(xNumTiles * yNumTiles));

    // Lock the GUI for drawing
    std::lock_guard drawingLock(view->drawingMutex);

    // Split up the rectangle into tiles and start rendering
    for (auto i = 0; i < xNumTiles; ++i) {
        for (auto j = 0; j < yNumTiles; ++j) {
            // TODO: Optimize with a static thread-pool
            tileFutures.push_back(std::async([this, rect, i, j, dpiScaleFactor, zoom] {
                this->rerenderTile(
                        {rect.x * dpiScaleFactor * zoom + i * tile_dim, rect.y * dpiScaleFactor * zoom + j * tile_dim},
                        {rect.x + i * (tile_dim / (static_cast<double>(dpiScaleFactor) * zoom)),
                         rect.y + j * (tile_dim / (static_cast<double>(dpiScaleFactor) * zoom))});
            }));
        }
    }

    // Wait for all tiles to render
    for (auto&& future: tileFutures) { future.wait(); }
}

void RenderJob::run() {
    // Get the global information for what areas to render
    bool rerenderComplete{false};
    std::vector<Rectangle<double>> rerenderRects;
    {
        std::lock_guard repaintRectLock(this->view->repaintRectMutex);

        rerenderComplete = this->view->rerenderComplete;
        this->view->rerenderComplete = false;
        rerenderRects = std::move(this->view->rerenderRects);
    }

    // If a full rerender is requested, we rerender using one rectangle that spans the whole page
    if (rerenderComplete) {
        Document* doc = this->view->xournal->getDocument();
        doc->lock();
        double pageWidth = view->page->getWidth();
        double pageHeight = view->page->getHeight();
        doc->unlock();

        rerenderRectangle({0, 0, pageWidth, pageHeight});
    } else {
        // TODO: Optimize by filtering intersections
        for (Rectangle<double> const& rect: rerenderRects) { rerenderRectangle(rect); }
    }

    // Schedule a repaint of the widget
    repaintWidget(this->view->getXournal()->getWidget());
}

/**
 * Repaint the widget in UI Thread
 */
void RenderJob::repaintWidget(GtkWidget* widget) {
    // "this" is not needed, "widget" is in
    // the closure, therefore no sync needed
    // Because of this the argument "widget" is needed
    Util::execInUiThread([=]() { gtk_widget_queue_draw(widget); });
}

auto RenderJob::getType() -> JobType { return JOB_TYPE_RENDER; }
