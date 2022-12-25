#include "RenderJob.h"

#include <cmath>    // for ceil, floor
#include <mutex>    // for mutex
#include <utility>  // for move
#include <vector>   // for vector

#include <cairo.h>  // for cairo_create, cairo_destroy, cairo_...
#include "gui/widgets/XournalWidget.h"  // for gtk_xournal_repaint_area

#include "control/Control.h"          // for Control
#include "control/ToolEnums.h"        // for TOOL_PLAY_OBJECT
#include "control/ToolHandler.h"      // for ToolHandler
#include "control/jobs/Job.h"         // for JOB_TYPE_RENDER, JobType
#include "gui/PageView.h"             // for XojPageView
#include "gui/XournalView.h"          // for XournalView
#include "model/Document.h"           // for Document
#include "util/Rectangle.h"           // for Rectangle
#include "util/Util.h"                // for execInUiThread
#include "util/raii/CairoWrappers.h"  // for CairoSurfaceSPtr, CairoSPtr
#include "view/DocumentView.h"        // for DocumentView

using xoj::util::Rectangle;

RenderJob::RenderJob(XojPageView* view): view(view) {}

auto RenderJob::getSource() -> void* { return this->view; }

void RenderJob::rerenderRectangle(Rectangle<double> const& rect) {
    const double ratio = view->xournal->getZoom() * this->view->xournal->getDpiScaleFactor();

    /**
     * The +1 makes sure the mask is big enough
     * For example, if rect.x = m + 0.9, rect.width = n + 0.2 and ratio = 1 and m and n are integers
     * We need a mask of width n+2 pixels for that...
     **/
    const auto x = std::floor(rect.x * ratio);
    const auto y = std::floor(rect.y * ratio);
    const auto width = int(std::ceil(rect.width * ratio)) + 1;
    const auto height = int(std::ceil(rect.height * ratio)) + 1;

    xoj::util::CairoSurfaceSPtr rectBuffer(cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height),
                                           xoj::util::adopt);
    cairo_surface_set_device_offset(rectBuffer.get(), -x, -y);
    cairo_surface_set_device_scale(rectBuffer.get(), ratio, ratio);

    renderToBuffer(rectBuffer.get());

    std::lock_guard lock(this->view->drawingMutex);
    xoj::util::CairoSPtr crPageBuffer(cairo_create(view->crBuffer.get()), xoj::util::adopt);

    cairo_set_operator(crPageBuffer.get(), CAIRO_OPERATOR_SOURCE);
    cairo_set_source_surface(crPageBuffer.get(), rectBuffer.get(), 0, 0);
    cairo_rectangle(crPageBuffer.get(), rect.x, rect.y, rect.width, rect.height);
    cairo_fill(crPageBuffer.get());
}

void RenderJob::run() {
    this->view->repaintRectMutex.lock();

    bool rerenderComplete = this->view->rerenderComplete;
    auto rerenderRects = std::move(this->view->rerenderRects);

    this->view->rerenderComplete = false;

    this->view->repaintRectMutex.unlock();

    const int dpiScaleFactor = this->view->xournal->getDpiScaleFactor();

    if (rerenderComplete) {
        const int dispWidth = this->view->getDisplayWidth() * dpiScaleFactor;
        const int dispHeight = this->view->getDisplayHeight() * dpiScaleFactor;
        const double ratio = this->view->xournal->getZoom() * dpiScaleFactor;

        xoj::util::CairoSurfaceSPtr newBuffer(cairo_image_surface_create(CAIRO_FORMAT_ARGB32, dispWidth, dispHeight),
                                              xoj::util::adopt);
        cairo_surface_set_device_scale(newBuffer.get(), ratio, ratio);

        renderToBuffer(newBuffer.get());

        {
            std::lock_guard lock(this->view->drawingMutex);
            std::swap(this->view->crBuffer, newBuffer);
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
    repaintWidgetArea(view->xournal->getWidget(), 0, 0, view->getWidth(), view->getHeight());
}

void RenderJob::repaintPageArea(double x1, double y1, double x2, double y2) const {
    double zoom = view->xournal->getZoom();
    int x = view->getX();
    int y = view->getY();
    repaintWidgetArea(view->xournal->getWidget(), x + std::floor(zoom * x1), y + std::floor(zoom * y1), x + std::ceil(zoom * x2), y + std::ceil(zoom * y2));
}

void RenderJob::renderToBuffer(cairo_surface_t* buffer) const {
    xoj::util::CairoSPtr crRect(cairo_create(buffer), xoj::util::adopt);

    DocumentView localView;
    localView.setMarkAudioStroke(this->view->getXournal()->getControl()->getToolHandler()->getToolType() ==
                                 TOOL_PLAY_OBJECT);
    localView.setPdfCache(this->view->xournal->getCache());

    std::lock_guard<Document> lock(*this->view->xournal->getDocument());
    localView.drawPage(this->view->page, crRect.get(), false);
}

auto RenderJob::getType() -> JobType { return JOB_TYPE_RENDER; }
