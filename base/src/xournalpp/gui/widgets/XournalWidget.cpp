#include "XournalWidget.h"

#include <cmath>
#include <utility>

#include <gdk/gdk.h>

XournalWidget::XournalWidget(const lager::reader<Settings>& settings, lager::reader<Viewport> viewport,
                             lager::context<ViewportAction> context):
        context(std::move(context)), viewport(std::move(viewport)) {
    this->drawingArea = gtk_drawing_scrollable_new();
    gtk_widget_set_hexpand(this->drawingArea, true);
    gtk_widget_set_vexpand(this->drawingArea, true);
    g_signal_connect(G_OBJECT(drawingArea), "size-allocate", G_CALLBACK(XournalWidget::sizeAllocateCallback), this);
    g_signal_connect(G_OBJECT(drawingArea), "realize", G_CALLBACK(XournalWidget::realizeCallback), this);
    g_signal_connect(G_OBJECT(drawingArea), "draw", G_CALLBACK(XournalWidget::drawCallback), this);
    g_signal_connect(G_OBJECT(drawingArea), "notify::hadjustment", G_CALLBACK(XournalWidget::initHScrolling), this);
    g_signal_connect(G_OBJECT(drawingArea), "notify::vadjustment", G_CALLBACK(XournalWidget::initVScrolling), this);

    GtkScrollable* scrollableWidget = GTK_SCROLLABLE(this->drawingArea);
    this->docMode = lager::reader<Settings::DocumentMode>{settings[&Settings::mode]};
    this->docMode.watch([](auto&&, auto&& mode) {
        /*
         * TODO
         * exchange DrawingManager, reallocate
         */
    });
    this->scrollX = lager::reader<double>{viewport[&Viewport::x]};
    this->scrollY = lager::reader<double>{viewport[&Viewport::y]};
    this->scrollX.watch([&](auto&&, auto&& x) {
        auto adj = gtk_scrollable_get_hadjustment(scrollableWidget);
        updateScrollbar(adj, x);
    });
    this->scrollY.watch([&](auto&&, auto&& y) {
        auto adj = gtk_scrollable_get_vadjustment(scrollableWidget);
        updateScrollbar(adj, y);
    });
    this->rawScale = lager::reader<double>{viewport[&Viewport::rawScale]};
    this->rawScale.watch([&](auto&&, auto&& v) { gtk_widget_queue_allocate(this->drawingArea); });
}

auto XournalWidget::initHScrolling(XournalWidget* self) -> void {
    GtkScrollable* scrollableWidget = GTK_SCROLLABLE(self->drawingArea);
    GtkAdjustment* hadjustment = gtk_scrollable_get_hadjustment(scrollableWidget);
    gtk_adjustment_configure(hadjustment, self->scrollX.get(), self->scrollX.get() - 150, self->scrollX.get() + 150,
                             STEP_INCREMENT, STEP_INCREMENT, 100);
    g_signal_connect(G_OBJECT(hadjustment), "value-changed", G_CALLBACK(XournalWidget::horizontalScroll), self);
}

auto XournalWidget::initVScrolling(XournalWidget* self) -> void {
    GtkScrollable* scrollableWidget = GTK_SCROLLABLE(self->drawingArea);
    GtkAdjustment* vadjustment = gtk_scrollable_get_vadjustment(scrollableWidget);
    gtk_adjustment_configure(vadjustment, self->scrollY.get(), self->scrollY.get() - 150, self->scrollY.get() + 150,
                             STEP_INCREMENT, STEP_INCREMENT, 100);
    g_signal_connect(G_OBJECT(vadjustment), "value-changed", G_CALLBACK(XournalWidget::verticalScroll), self);
}

auto XournalWidget::sizeAllocateCallback(GtkWidget* drawingArea, GdkRectangle* allocation, XournalWidget* self)
        -> void {
    if (allocation->width != self->viewport->width || allocation->height != self->viewport->height)
        self->context.dispatch(Resize{allocation->width, allocation->height});

    GtkScrollable* scrollableWidget = GTK_SCROLLABLE(drawingArea);
    GtkAdjustment* hadjustment = gtk_scrollable_get_hadjustment(scrollableWidget);
    GtkAdjustment* vadjustment = gtk_scrollable_get_vadjustment(scrollableWidget);

    if (self->docMode.get() == Settings::INFINITE) {
        gtk_adjustment_set_lower(vadjustment, -1.5 * allocation->height);
        gtk_adjustment_set_upper(vadjustment, 1.5 * allocation->height);
        gtk_adjustment_set_lower(hadjustment, -1.5 * allocation->width);
        gtk_adjustment_set_upper(hadjustment, 1.5 * allocation->width);
    } else {
        gtk_adjustment_set_lower(vadjustment, 0);
        // gtk_adjustment_set_upper(vadjustment, self->layout->documentHeight * self->viewport->rawScale);
        gtk_adjustment_set_lower(hadjustment, 0);
        // gtk_adjustment_set_upper(hadjustment, self->layout->documentWidth * self->viewport->rawScale);
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
    /*auto context = self->renderer->getGtkStyleContext();
    gtk_render_background(context, cr, x1, y1, x2 - x1, y2 - y1);*/

    // cairo clip is relative to viewport position
    // Rectangle<double> clippingRect(self->viewport->x + x1, self->viewport->y + y1, x2 - x1, y2 - y1);

    return true;
}

auto XournalWidget::updateScrollbar(GtkAdjustment* adj, double value) -> void {
    if (docMode.get() == Settings::DocumentMode::INFINITE) {
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
    self->context.dispatch(Scroll{Scroll::HORIZONTAL, xDiff});
}

auto XournalWidget::verticalScroll(GtkAdjustment* vadjustment, XournalWidget* self) -> void {
    double yDiff = gtk_adjustment_get_value(vadjustment);
    self->context.dispatch(Scroll{Scroll::VERTICAL, yDiff});
}

auto XournalWidget::getGtkWidget() -> GtkWidget* { return this->drawingArea; }