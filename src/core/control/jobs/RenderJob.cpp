#include "RenderJob.h"

#include <cmath>    // for ceil, floor
#include <mutex>    // for mutex
#include <utility>  // for move
#include <vector>   // for vector

#include <cairo.h>  // for cairo_create, cairo_destroy, cairo_...

#include "control/Control.h"            // for Control
#include "control/ToolEnums.h"          // for TOOL_PLAY_OBJECT
#include "control/ToolHandler.h"        // for ToolHandler
#include "control/jobs/Job.h"           // for JOB_TYPE_RENDER, JobType
#include "gui/PageView.h"               // for XojPageView
#include "gui/XournalView.h"            // for XournalView
#include "gui/widgets/XournalWidget.h"  // for gtk_xournal_repaint_area
#include "model/Document.h"             // for Document
#include "model/XojPage.h"              // for Page
#include "util/Rectangle.h"             // for Rectangle
#include "util/Util.h"                  // for execInUiThread
#include "util/raii/CairoWrappers.h"    // for CairoSurfaceSPtr, CairoSPtr
#include "view/DocumentView.h"          // for DocumentView
#include "view/Mask.h"                  // for Mask

using xoj::util::Rectangle;

RenderJob::RenderJob(XojPageView* view): view(view) {}

auto RenderJob::getSource() -> void* { return this->view; }

void RenderJob::rerenderRectangle(Rectangle<double> const& rect) {
    /**
     * Padding seems to be necessary to prevent artefacts of most strokes.
     * These artefacts are most pronounced when using the stroke deletion
     * tool on ellipses, but also occur occasionally when removing regular
     * strokes.
     **/
    constexpr int RENDER_PADDING = 1;

    Range maskRange(rect);
    maskRange.addPadding(RENDER_PADDING);
    xoj::view::Mask newMask(view->xournal->getDpiScaleFactor(), maskRange, view->xournal->getZoom(),
                            CAIRO_CONTENT_COLOR_ALPHA);

    renderToBuffer(newMask.get());

    std::lock_guard lock(this->view->drawingMutex);
    assert(view->buffer.isInitialized());

    newMask.paintTo(view->buffer.get());
}

void RenderJob::run() {
    this->view->repaintRectMutex.lock();

    bool rerenderComplete = this->view->rerenderComplete;
    auto rerenderRects = std::move(this->view->rerenderRects);

    this->view->rerenderComplete = false;

    this->view->repaintRectMutex.unlock();

    if (rerenderComplete) {
        xoj::view::Mask newMask(view->xournal->getDpiScaleFactor(),
                                Range(0, 0, view->page->getWidth(), view->page->getHeight()), view->xournal->getZoom(),
                                CAIRO_CONTENT_COLOR_ALPHA);

        renderToBuffer(newMask.get());
        {
            std::lock_guard lock(this->view->drawingMutex);
            std::swap(this->view->buffer, newMask);
        }
        repaintPage();
    } else {
        for (Rectangle<double> const& rect: rerenderRects) {
            rerenderRectangle(rect);
            repaintPageArea(rect.x, rect.y, rect.x + rect.width, rect.y + rect.height);
        }
    }
}

static void repaintWidgetArea(GtkWidget* widget, int x1, int y1, int x2, int y2) {
    Util::execInUiThread([=]() { gtk_xournal_repaint_area(widget, x1, y1, x2, y2); });
}

void RenderJob::repaintPage() const {
    repaintPageArea(0, 0, view->getWidth(), view->getHeight());
}

void RenderJob::repaintPageArea(double x1, double y1, double x2, double y2) const {
    double zoom = view->xournal->getZoom();
    int x = view->getX();
    int y = view->getY();
    repaintWidgetArea(view->xournal->getWidget(), x + std::floor(zoom * x1), y + std::floor(zoom * y1), x + std::ceil(zoom * x2), y + std::ceil(zoom * y2));
}

void RenderJob::renderToBuffer(cairo_t* cr) const {
    DocumentView localView;
    localView.setMarkAudioStroke(this->view->getXournal()->getControl()->getToolHandler()->getToolType() ==
                                 TOOL_PLAY_OBJECT);
    localView.setPdfCache(this->view->xournal->getCache());

    std::lock_guard<Document> lock(*this->view->xournal->getDocument());
    localView.drawPage(this->view->page, cr, false);
}

auto RenderJob::getType() -> JobType { return JOB_TYPE_RENDER; }
