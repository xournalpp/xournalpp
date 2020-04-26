#include "XournalWidget.h"

#include <cmath>

#include <control/Dispatcher.h>
#include <gdk/gdk.h>

#include "gui/Renderer.h"
#include "gui/Shadow.h"
#include "gui/inputdevices/InputContext.h"

XournalWidget::XournalWidget(std::unique_ptr<Renderer> render, std::shared_ptr<Viewport> viewport,
                             std::shared_ptr<Layout> layout):
        renderer(std::move(render)), viewport(std::move(viewport)), layout(std::move(layout)) {
    this->drawingArea = gtk_drawing_scrollable_new();
    gtk_widget_set_hexpand(this->drawingArea, true);
    gtk_widget_set_vexpand(this->drawingArea, true);
    g_signal_connect(G_OBJECT(drawingArea), "size-allocate", G_CALLBACK(XournalWidget::sizeAllocateCallback), this);
    g_signal_connect(G_OBJECT(drawingArea), "realize", G_CALLBACK(XournalWidget::realizeCallback), this);
    g_signal_connect(G_OBJECT(drawingArea), "draw", G_CALLBACK(XournalWidget::drawCallback), this);
    g_signal_connect(G_OBJECT(drawingArea), "notify::hadjustment", G_CALLBACK(XournalWidget::initHScrolling), this);
    g_signal_connect(G_OBJECT(drawingArea), "notify::vadjustment", G_CALLBACK(XournalWidget::initVScrolling), this);
    viewport->registerListener(*this, [](auto e) { return true; });
    layout->registerListener(*this, [](auto e) { return true; });
}

XournalWidget::~XournalWidget() { gtk_widget_destroy(this->drawingArea); };

auto XournalWidget::initHScrolling(XournalWidget* self) -> void {
    GtkScrollable* scrollableWidget = GTK_SCROLLABLE(self->drawingArea);
    GtkAdjustment* hadjustment = gtk_scrollable_get_hadjustment(scrollableWidget);
    gtk_adjustment_configure(hadjustment, self->viewport->getX(), self->viewport->getX() - 150,
                             self->viewport->getX() + 150, STEP_INCREMENT, STEP_INCREMENT, 100);
    g_signal_connect(G_OBJECT(hadjustment), "value-changed", G_CALLBACK(XournalWidget::horizontalScroll), self);
}

auto XournalWidget::initVScrolling(XournalWidget* self) -> void {
    GtkScrollable* scrollableWidget = GTK_SCROLLABLE(self->drawingArea);
    GtkAdjustment* vadjustment = gtk_scrollable_get_vadjustment(scrollableWidget);
    gtk_adjustment_configure(vadjustment, self->viewport->getY(), self->viewport->getY() - 150,
                             self->viewport->getY() + 150, STEP_INCREMENT, STEP_INCREMENT, 100);
    g_signal_connect(G_OBJECT(vadjustment), "value-changed", G_CALLBACK(XournalWidget::verticalScroll), self);
}

auto XournalWidget::sizeAllocateCallback(GtkWidget* drawingArea, GdkRectangle* allocation, XournalWidget* self)
        -> void {
    if (allocation->width != self->viewport->getWidth() || allocation->height != self->viewport->getHeight())
        Dispatcher::getMainStage().dispatch(Allocation{allocation->width, allocation->height});

    Rectangle<double> documentSize = self->layout->getDocumentSize();
    GtkScrollable* scrollableWidget = GTK_SCROLLABLE(drawingArea);
    GtkAdjustment* hadjustment = gtk_scrollable_get_hadjustment(scrollableWidget);
    GtkAdjustment* vadjustment = gtk_scrollable_get_vadjustment(scrollableWidget);

    if (self->layout->isInfiniteVertically()) {
        gtk_adjustment_set_lower(vadjustment, -1.5 * allocation->height);
        gtk_adjustment_set_upper(vadjustment, 1.5 * allocation->height);
    } else {
        gtk_adjustment_set_lower(vadjustment, documentSize.y);
        gtk_adjustment_set_upper(vadjustment, documentSize.y + documentSize.height * self->viewport->getRawScale());
    }
    if (self->layout->isInfiniteHorizontally()) {
        gtk_adjustment_set_lower(hadjustment, -1.5 * allocation->width);
        gtk_adjustment_set_upper(hadjustment, 1.5 * allocation->width);
    } else {
        gtk_adjustment_set_lower(hadjustment, documentSize.x);
        gtk_adjustment_set_upper(hadjustment, documentSize.x + documentSize.width * self->viewport->getRawScale());
    }
    gtk_adjustment_set_page_size(vadjustment, allocation->height);
    gtk_adjustment_set_page_size(hadjustment, allocation->width);
    gtk_adjustment_set_page_increment(vadjustment, allocation->height - STEP_INCREMENT);
    gtk_adjustment_set_page_increment(hadjustment, allocation->width - STEP_INCREMENT);

    // gtk_widget_queue_draw(drawingArea); TODO?
}

auto XournalWidget::realizeCallback(GtkWidget* drawingArea, XournalWidget* self) -> void {
    // Disable event compression
    gdk_window_set_event_compression(gtk_widget_get_window(drawingArea), false);
}

auto XournalWidget::drawCallback(GtkWidget* drawArea, cairo_t* cr, XournalWidget* self) -> gboolean {
    double x1 = NAN, x2 = NAN, y1 = NAN, y2 = NAN;
    cairo_clip_extents(cr, &x1, &y1, &x2, &y2);

    // render background
    auto context = self->renderer->getGtkStyleContext();
    gtk_render_background(context, cr, x1, y1, x2 - x1, y2 - y1);

    // cairo clip is relative to viewport position
    Rectangle<double> clippingRect(self->viewport->getX() + x1, self->viewport->getY() + y1, x2 - x1, y2 - y1);

    Rectangle<double> documentSize = self->layout->getDocumentSize();
    bool hInfinite = self->layout->isInfiniteHorizontally();
    bool vInfinite = self->layout->isInfiniteVertically();
    int allocWidth = gtk_widget_get_allocated_width(drawArea);
    int allocHeight = gtk_widget_get_allocated_height(drawArea);

    // if width / height of document (multiplied by scale) is smaller than widget width translate the cairo context
    if (!hInfinite && documentSize.width * self->viewport->getRawScale() < allocWidth) {
        double borderWidth = (allocWidth - documentSize.width) / 2;
        clippingRect.width = std::min(clippingRect.width, documentSize.width * self->viewport->getRawScale());
        cairo_translate(cr, borderWidth, 0);
    }
    if (!vInfinite && documentSize.height * self->viewport->getRawScale() < allocHeight) {
        double borderHeight = (allocHeight - documentSize.height) / 2;
        clippingRect.height = std::min(clippingRect.height, documentSize.height * self->viewport->getRawScale());
        cairo_translate(cr, 0, borderHeight);
    }

    // transfer state to ideally stateless renderer
    self->renderer->render(cr, self->viewport);
    return true;
}

auto XournalWidget::updateScrollbar(ScrollEvent::ScrollDirection direction, double value, bool infinite) -> void {
    GtkAdjustment* adj = nullptr;
    GtkScrollable* scrollableWidget = GTK_SCROLLABLE(this->drawingArea);
    if (direction == ScrollEvent::HORIZONTAL)
        adj = gtk_scrollable_get_hadjustment(scrollableWidget);
    else
        adj = gtk_scrollable_get_vadjustment(scrollableWidget);
    if (infinite) {
        double upper = gtk_adjustment_get_upper(adj);
        double lower = gtk_adjustment_get_lower(adj);
        double fullRange = upper - lower;
        double lowerThreshhold = lower + 0.1 * fullRange;
        double upperThreshhold = upper - 0.1 * fullRange;
        if (value < lowerThreshhold) {
            gtk_adjustment_set_lower(adj, lower - 0.2 * fullRange);
            gtk_adjustment_set_upper(adj, upper - 0.2 * fullRange);
        } else if (value > upperThreshhold) {
            gtk_adjustment_set_lower(adj, lower + 0.2 * fullRange);
            gtk_adjustment_set_upper(adj, upper + 0.2 * fullRange);
        }
    }
}

auto XournalWidget::horizontalScroll(GtkAdjustment* hadjustment, XournalWidget* self) -> void {
    double xDiff = gtk_adjustment_get_value(hadjustment);
    HScroll horizontalScrollAction{xDiff};
    Dispatcher::getMainStage().dispatch(horizontalScrollAction);
}

auto XournalWidget::verticalScroll(GtkAdjustment* vadjustment, XournalWidget* self) -> void {
    double yDiff = gtk_adjustment_get_value(vadjustment);
    VScroll verticalScrollAction{yDiff};
    Dispatcher::getMainStage().dispatch(verticalScrollAction);
}

auto XournalWidget::eventCallback(const ViewportEvent& event) -> void {
    if (typeid(event) == typeid(ScrollEvent)) {
        auto scrollEvent = dynamic_cast<const ScrollEvent&>(event);
        updateScrollbar(scrollEvent.getDirection(), scrollEvent.getDifference(), layout->isInfiniteHorizontally());
    }
    gtk_widget_queue_allocate(this->drawingArea);
}

auto XournalWidget::eventCallback(const LayoutEvent& event) -> void { gtk_widget_queue_allocate(this->drawingArea); }

auto XournalWidget::getGtkWidget() -> GtkWidget* { return this->drawingArea; };