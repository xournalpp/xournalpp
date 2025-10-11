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
#include "view/Tiling.h"                // for Tiling

#include "config-debug.h"     // for DEBUG_TIME_RENDER_LOOPS
#include "config-features.h"  // for RENDER_ALGORITHM

#define CAIRO_TEE_SURFACE 0
#define SEQUENCIAL_LOOP 1
#define PARALLEL_LOOP 2

#if RENDER_ALGORITHM == PARALLEL_LOOP
#include <execution>
#elif RENDER_ALGORITHM == CAIRO_TEE_SURFACE
#include <cairo-tee.h>
#endif

#if defined(__has_cpp_attribute) && __has_cpp_attribute(likely)
#define XOJ_CPP20_UNLIKELY [[unlikely]]
#else
#define XOJ_CPP20_UNLIKELY
#endif

using xoj::util::Rectangle;

RenderJob::RenderJob(XojPageView* view): view(view) {}

auto RenderJob::getSource() -> void* { return this->view; }

static void renderToBuffer(XojPageView* view, cairo_t* cr, bool parallelExecution) {
    DocumentView localView;
    localView.setMarkAudioStroke(view->getXournal()->getControl()->getToolHandler()->getToolType() == TOOL_PLAY_OBJECT);
    localView.setPdfCache(view->getXournal()->getCache());

    localView.drawPage(view->getPage(), cr, false, xoj::view::BACKGROUND_SHOW_ALL, parallelExecution);
}

void RenderJob::rerenderRectangle(Rectangle<double> const& rect) {
    /**
     * Padding seems to be necessary to prevent artefacts of most strokes.
     * These artefacts are most pronounced when using the stroke deletion
     * tool on ellipses, but also occur occasionally when removing regular
     * strokes.
     **/
    constexpr int RENDER_PADDING = 1;

    const double zoom = view->xournal->getZoom();
    Range maskRange(rect);
    maskRange.addPadding(RENDER_PADDING);
    xoj::view::Mask newMask(view->xournal->getDpiScaleFactor(), maskRange, zoom, CAIRO_CONTENT_COLOR_ALPHA);

    {
        std::shared_lock<Document> lock(*this->view->xournal->getDocument());
        renderToBuffer(this->view, newMask.get(), false);
    }

    std::lock_guard lock(this->view->drawingMutex);
    for (auto& t: view->tiles.getTilesFor(Range(0, 0, view->page->getWidth(), view->page->getHeight()))) {
        newMask.paintTo(t->get());
    }
}

static void renderToTiles(XojPageView* view, xoj::view::Tiling& tiles) {
#if RENDER_ALGORITHM == CAIRO_TEE_SURFACE
#define RENDER_ALGORITHM_STRING "Cairo Tee Surface"
#ifdef DEBUG_TIME_RENDER_LOOPS
    auto t = g_get_monotonic_time();
#endif
    xoj::util::Rectangle<int> e = tiles.getTiles().front()->getExtent();
    for (auto&& t: tiles.getTiles()) {
        cairo_surface_set_device_offset(cairo_get_target(t->get()), -t->getExtent().x, -t->getExtent().y);
        e.unite(t->getExtent());
    }
    printf("e: %d x %d + (%d ; %d)\n", e.width, e.height, e.x, e.y);

    // The tee surface takes its extent from its "primary surface". To render it all, we create a dummy surface
    // with the right extent. The recording surface is the fastest I could find (but it does take a bit of time)
    cairo_rectangle_t r{static_cast<double>(e.x), static_cast<double>(e.y), static_cast<double>(e.width),
                        static_cast<double>(e.height)};
    auto* rec = cairo_recording_surface_create(CAIRO_CONTENT_COLOR_ALPHA, &r);

    auto* tee = cairo_tee_surface_create(rec);
    cairo_surface_destroy(rec);  // Now owned by tee
    for (auto&& t: tiles.getTiles()) {
        cairo_tee_surface_add(tee, cairo_get_target(t->get()));
    }
    cairo_t* c = cairo_create(tee);
    cairo_surface_destroy(tee);  // Now owned by c

    cairo_scale(c, tiles.getZoom(), tiles.getZoom());
    {
#ifdef DEBUG_TIME_RENDER_LOOPS
        auto t2 = g_get_monotonic_time();
#endif
        std::shared_lock<Document> lock(*view->getXournal()->getDocument());
#ifdef DEBUG_TIME_RENDER_LOOPS
        t += g_get_monotonic_time() - t2;  // Do not count the mutex locking time
#endif
        renderToBuffer(view, c, false);
    }
    cairo_destroy(c);

    for (auto&& t: tiles.getTiles()) {
        cairo_surface_set_device_offset(cairo_get_target(t->get()), 0, 0);
    }
#else
#ifdef DEBUG_TIME_RENDER_LOOPS
    gint64 t;
#endif
    {
        std::shared_lock<Document> lock(*view->getXournal()->getDocument());
#ifdef DEBUG_TIME_RENDER_LOOPS
        t = g_get_monotonic_time();
#endif
#if RENDER_ALGORITHM == PARALLEL_LOOP
#define RENDER_ALGORITHM_STRING "Parallel loop"
        // We lock the document only once and render the tiles in parallel. This is safe because rendering the
        // document does not modify it and never invalidates any iterators
        std::for_each(std::execution::par, tiles.getTiles().begin(), tiles.getTiles().end(),
                      [view](auto&& t) { renderToBuffer(view, t->get(), true); });
#else  // RENDER_ALGORITHM==SEQUENCIAL_LOOP
#define RENDER_ALGORITHM_STRING "Sequencial loop"
        std::for_each(tiles.getTiles().begin(), tiles.getTiles().end(),
                      [view](auto&& t) { renderToBuffer(view, t->get(), false); });
#endif
    }
#endif
#ifdef DEBUG_TIME_RENDER_LOOPS
    t = g_get_monotonic_time() - t;
    printf(u8"%s: Rendered %2zu tiles in %8ld µs (zoom %f)\n", RENDER_ALGORITHM_STRING, tiles.getTiles().size(), t,
           view->getZoom());
#endif
}

void RenderJob::run() {
    this->view->rerenderDataMutex.lock();

    bool rerenderComplete = std::exchange(this->view->rerenderData.rerenderComplete, false);
    bool sizeChanged = std::exchange(this->view->rerenderData.sizeChanged, false);
    auto rerenderRects = std::move(this->view->rerenderData.rerenderRects);
    auto retiling = std::move(this->view->rerenderData.retiling);
    auto center = this->view->rerenderData.centerOfVisibleArea;  // Do not move out - it may still be used.
    auto mustRenderRadius = this->view->rerenderData.mustRenderRadius;

    this->view->rerenderDataMutex.unlock();

    if (!rerenderComplete) {
        for (Rectangle<double> const& rect: rerenderRects) {
            // The number of rectangles is typically very small (1 or 2) so no need to optimize/parallelize this loop
            rerenderRectangle(rect);
            repaintPageArea(rect.x, rect.y, rect.x + rect.width, rect.y + rect.height);
        }
        if (!retiling.missingTiles.empty()) {
            xoj::view::Tiling newTiles;
            newTiles.setZoom(view->xournal->getZoom());
            newTiles.createTiles(view->xournal->getDpiScaleFactor(), std::move(retiling));
            std::vector<xoj::util::Rectangle<int>> toRepaint;
            toRepaint.reserve(newTiles.getTiles().size());

            renderToTiles(this->view, newTiles);

            for (auto&& t: newTiles.getTiles()) {
                toRepaint.emplace_back(t->getExtent());
            }
            {
                std::lock_guard lock(this->view->drawingMutex);
                this->view->tiles.append(newTiles);
            }
            for (auto&& t: toRepaint) {
                repaintTile(t);
            }
        }
    } else {
        xoj::view::Tiling newTiles;
        newTiles.populate(view->xournal->getDpiScaleFactor(), center,
                          Range(0, 0, view->page->getWidth(), view->page->getHeight()), mustRenderRadius,
                          view->xournal->getZoom(), std::move(retiling.unusedTiles));

        renderToTiles(this->view, newTiles);

        {
            std::lock_guard lock(this->view->drawingMutex);
            std::swap(this->view->tiles, newTiles);
        }
        if (sizeChanged) {
            // We do not have any control on what portion of the widget needs to be redrawn. Redraw it all.
            Util::execInUiThread([w = view->xournal->getWidget()]() { gtk_widget_queue_draw(w); });
        } else {
            repaintPage();
        }
    }
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

void RenderJob::repaintTile(const xoj::util::Rectangle<int>& area) const {
    auto p = this->view->getPixelPosition();
    repaintWidgetArea(view->xournal->getWidget(), p.x + area.x, p.y + area.y, p.x + area.x + area.width,
                      p.y + area.y + area.height);
}

auto RenderJob::getType() -> JobType { return JOB_TYPE_RENDER; }
