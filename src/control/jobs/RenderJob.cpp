#include "RenderJob.h"

#include <cmath>

#include "control/Control.h"
#include "control/ToolHandler.h"
#include "gui/PageView.h"
#include "gui/XournalView.h"
#include "model/Document.h"
#include "view/DocumentView.h"
#include "view/PdfView.h"

#include "Rectangle.h"
#include "Util.h"

RenderJob::RenderJob(XojPageView* view): view(view) {}

auto RenderJob::getSource() -> void* { return this->view; }

void RenderJob::rerenderRectangle(Rectangle<double> const& rect) {
    double zoom = view->xournal->getZoom();
    Document* doc = view->xournal->getDocument();
    doc->lock();
    double pageWidth = view->page->getWidth();
    double pageHeight = view->page->getHeight();
    doc->unlock();

    auto x = int(std::lround(rect.x * zoom));
    auto y = int(std::lround(rect.y * zoom));
    auto width = int(std::lround(rect.width * zoom));
    auto height = int(std::lround(rect.height * zoom));

    cairo_surface_t* rectBuffer = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    cairo_t* crRect = cairo_create(rectBuffer);
    cairo_translate(crRect, -x, -y);
    cairo_scale(crRect, zoom, zoom);

    DocumentView v;
    Control* control = view->getXournal()->getControl();
    v.setMarkAudioStroke(control->getToolHandler()->getToolType() == TOOL_PLAY_OBJECT);
    v.limitArea(rect.x, rect.y, rect.width, rect.height);

    bool backgroundVisible = view->page->isLayerVisible(0);
    if (backgroundVisible && view->page->getBackgroundType().isPdfPage()) {
        int pgNo = view->page->getPdfPageNr();
        XojPdfPageSPtr popplerPage = doc->getPdfPage(pgNo);
        PdfCache* cache = view->xournal->getCache();
        PdfView::drawPage(cache, popplerPage, crRect, zoom, pageWidth, pageHeight);
    }

    doc->lock();
    v.drawPage(view->page, crRect, false);
    doc->unlock();

    cairo_destroy(crRect);

    g_mutex_lock(&view->drawingMutex);

    cairo_t* crPageBuffer = cairo_create(view->crBuffer);

    cairo_set_operator(crPageBuffer, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_surface(crPageBuffer, rectBuffer, x, y);
    cairo_rectangle(crPageBuffer, x, y, width, height);
    cairo_fill(crPageBuffer);

    cairo_destroy(crPageBuffer);

    cairo_surface_destroy(rectBuffer);

    g_mutex_unlock(&view->drawingMutex);
}

void RenderJob::run() {
    double zoom = this->view->xournal->getZoom();

    g_mutex_lock(&this->view->repaintRectMutex);

    bool rerenderComplete = this->view->rerenderComplete;
    auto rerenderRects = std::move(this->view->rerenderRects);

    this->view->rerenderComplete = false;

    g_mutex_unlock(&this->view->repaintRectMutex);

    int dpiScaleFactor = this->view->xournal->getDpiScaleFactor();

    if (rerenderComplete || dpiScaleFactor > 1) {
        Document* doc = this->view->xournal->getDocument();

        int dispWidth = this->view->getDisplayWidth();
        int dispHeight = this->view->getDisplayHeight();

        dispWidth *= dpiScaleFactor;
        dispHeight *= dpiScaleFactor;
        zoom *= dpiScaleFactor;

        cairo_surface_t* crBuffer = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, dispWidth, dispHeight);
        cairo_t* cr2 = cairo_create(crBuffer);
        cairo_scale(cr2, zoom, zoom);

        XojPdfPageSPtr popplerPage;

        doc->lock();

        if (this->view->page->getBackgroundType().isPdfPage()) {
            int pgNo = this->view->page->getPdfPageNr();
            popplerPage = doc->getPdfPage(pgNo);
        }

        Control* control = view->getXournal()->getControl();
        DocumentView localView;
        localView.setMarkAudioStroke(control->getToolHandler()->getToolType() == TOOL_PLAY_OBJECT);
        int width = this->view->page->getWidth();
        int height = this->view->page->getHeight();

        bool backgroundVisible = this->view->page->isLayerVisible(0);
        if (backgroundVisible) {
            PdfView::drawPage(this->view->xournal->getCache(), popplerPage, cr2, zoom, width, height);
        }
        localView.drawPage(this->view->page, cr2, false);

        cairo_destroy(cr2);

        g_mutex_lock(&this->view->drawingMutex);

        if (this->view->crBuffer) {
            cairo_surface_destroy(this->view->crBuffer);
        }
        this->view->crBuffer = crBuffer;

        g_mutex_unlock(&this->view->drawingMutex);
        doc->unlock();
    } else {
        for (Rectangle<double> const& rect: rerenderRects) {
            rerenderRectangle(rect);
        }
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
