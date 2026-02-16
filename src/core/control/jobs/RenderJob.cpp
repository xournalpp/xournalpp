#include "RenderJob.h"

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
#include "util/Assert.h"                // for xoj_assert
#include "util/Rectangle.h"             // for Rectangle
#include "util/Util.h"                  // for execInUiThread
#include "util/raii/CairoWrappers.h"    // for CairoSurfaceSPtr, CairoSPtr
#include "util/safe_casts.h"            // for strict_cast, as_signed, as_si...
#include "view/DocumentView.h"          // for DocumentView
#include "view/Mask.h"                  // for Mask
#include "view/QuadPdfCache.h"          // for QuadPdfCache

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

    xoj::view::DocumentView localView;
    localView.setMarkAudioStroke(this->view->getXournal()->getControl()->getToolHandler()->getToolType() ==
                                 TOOL_PLAY_OBJECT);
    localView.setPdfCache(this->view->xournal->getCache());

    std::shared_lock<Document> doclock(*this->view->xournal->getDocument(), std::defer_lock);
    std::unique_lock lock(this->view->drawingMutex, std::defer_lock);
    std::lock(doclock, lock);  // Lock both mutexes at once to avoid deadlocks

    for (cairo_t* cr: this->view->pixelCache.getSurfacesFor(maskRange)) {
        xoj::util::CairoSaveGuard guard(cr);
        cairo_rectangle(cr, maskRange.minX, maskRange.minY, maskRange.getWidth(), maskRange.getHeight());
        cairo_clip(cr);
        localView.drawPage(this->view->page, cr, false);
    }
}

void RenderJob::run() {
    this->view->repaintRectMutex.lock();
    auto rerenderRects = std::move(this->view->rerenderRects);
    auto tiles = std::move(this->view->tilesToRender);
    this->view->repaintRectMutex.unlock();

    for (Rectangle<double> const& rect: rerenderRects) {
        rerenderRectangle(rect);
    }

    xoj::view::DocumentView localView;
    localView.setMarkAudioStroke(view->getXournal()->getControl()->getToolHandler()->getToolType() == TOOL_PLAY_OBJECT);
    xoj::view::BackgroundFlags flags = xoj::view::BACKGROUND_SHOW_ALL;
    flags.showPDF = xoj::view::HIDE_PDF_BACKGROUND;  // Already printed (if any)

    for (auto&& tileinfo: tiles) {
        auto mask = this->view->pixelCache.makeSuitableMask(tileinfo);
        std::shared_lock<Document> lock(*this->view->xournal->getDocument());
        ConstPageRef p = this->view->getPage();
        if (auto n = p->getPdfPageNr(); p->isLayerVisible(0) && n != npos) {
            // There is a visible PDF background
            // As we are making a tile for the quad tree, we most likely can use a single tile of the PdfCache
            // This optimizes-out rendering of extra tiles in the PdfCache due to rounding phenomenons
            this->view->xournal->getCache()->paintSingleTile(mask->get(), n, tileinfo, p->getWidth(), p->getHeight());
        }
        localView.drawPage(this->view->page, mask->get(), false, flags);
        {
            std::lock_guard lock(this->view->drawingMutex);
            this->view->pixelCache.assignBufferToNode(std::move(mask), tileinfo);
        }
    }
    repaintPage();
}

static void repaintWidgetArea(GtkWidget* widget, int x1, int y1, int x2, int y2) {
    Util::execInUiThread([=]() { gtk_xournal_repaint_area(widget, x1, y1, x2, y2); });
}

void RenderJob::repaintPage() const { repaintPageArea(0, 0, view->getWidth(), view->getHeight()); }

void RenderJob::repaintPageArea(double x1, double y1, double x2, double y2) const {
    double zoom = view->xournal->getZoom();
    auto p = this->view->getPixelPosition();
    repaintWidgetArea(view->xournal->getWidget(), p.x + floor_cast<int>(zoom * x1), p.y + floor_cast<int>(zoom * y1),
                      p.x + ceil_cast<int>(zoom * x2), p.y + ceil_cast<int>(zoom * y2));
}

auto RenderJob::getType() -> JobType { return JOB_TYPE_RENDER; }
