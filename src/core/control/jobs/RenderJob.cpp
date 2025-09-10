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

#if defined(__has_cpp_attribute) && __has_cpp_attribute(likely)
#define XOJ_CPP20_UNLIKELY [[unlikely]]
#else
#define XOJ_CPP20_UNLIKELY
#endif

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
    if (!view->buffer.isInitialized()) {
        // Todo: the buffer must not be uninitializable here, either by moving it into the job or by locking it at job
        // creation a shared prt may also be suffice.
        XOJ_CPP20_UNLIKELY return;
    }
    newMask.paintTo(view->buffer.get());
}

void RenderJob::run() {
    this->view->repaintRectMutex.lock();

    bool rerenderComplete = std::exchange(this->view->rerenderComplete, false);
    bool sizeChanged = std::exchange(this->view->sizeChanged, false);
    auto rerenderRects = std::move(this->view->rerenderRects);

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
        if (sizeChanged) {
            // We do not have any control on what portion of the widget needs to be redrawn. Redraw it all.
            Util::execInUiThread([w = view->xournal->getWidget()]() { gtk_widget_queue_draw(w); });
        } else {
            repaintPage();
        }
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

void RenderJob::repaintPage() const { repaintPageArea(0, 0, view->getWidth(), view->getHeight()); }

void RenderJob::repaintPageArea(double x1, double y1, double x2, double y2) const {
    double zoom = view->xournal->getZoom();
    int x = view->getX();
    int y = view->getY();
    repaintWidgetArea(view->xournal->getWidget(), x + floor_cast<int>(zoom * x1), y + floor_cast<int>(zoom * y1),
                      x + ceil_cast<int>(zoom * x2), y + ceil_cast<int>(zoom * y2));
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
