#include "XournalWidget.h"

#include <cmath>

#include <gdk/gdk.h>

#include "gui/Renderer.h"
#include "gui/Shadow.h"
#include "gui/inputdevices/InputContext.h"

XournalWidget::XournalWidget(std::shared_ptr<InputContext> inputContext, std::shared_ptr<Renderer> render):
        input(std::move(inputContext)), renderer(std::move(render)) {
    this->x = renderer->getDocumentSize().x;
    this->y = renderer->getDocumentSize().y;
    this->init();
}

auto XournalWidget::init() -> void {
    this->drawingArea = gtk_drawing_scrollable_new();
    gtk_widget_set_hexpand(this->drawingArea, true);
    gtk_widget_set_vexpand(this->drawingArea, true);
    g_signal_connect(G_OBJECT(drawingArea), "size-allocate", G_CALLBACK(XournalWidget::sizeAllocateCallback), this);
    g_signal_connect(G_OBJECT(drawingArea), "realize", G_CALLBACK(XournalWidget::realizeCallback), this);
    g_signal_connect(G_OBJECT(drawingArea), "draw", G_CALLBACK(XournalWidget::drawCallback), this);
    g_signal_connect(G_OBJECT(drawingArea), "notify::hadjustment", G_CALLBACK(XournalWidget::initHScrolling), this);
    g_signal_connect(G_OBJECT(drawingArea), "notify::vadjustment", G_CALLBACK(XournalWidget::initVScrolling), this);
    if (this->input) {
        this->input->connect(this);
    }
}

XournalWidget::~XournalWidget() { gtk_widget_destroy(this->drawingArea); };

auto XournalWidget::getGtkWidget() -> GtkWidget* { return this->drawingArea; }

auto XournalWidget::getVisibleArea() -> Rectangle<double> {
    Rectangle<double> visibleArea{};
    visibleArea.x = x;
    visibleArea.y = y;
    visibleArea.width = static_cast<double>(gtk_widget_get_allocated_width(this->drawingArea)) * scale;
    visibleArea.height = static_cast<double>(gtk_widget_get_allocated_height(this->drawingArea)) * scale;
    return visibleArea;
}

auto inline XournalWidget::queueRedraw() -> void { gtk_widget_queue_draw(this->drawingArea); }

auto inline XournalWidget::queueAllocate() -> void { gtk_widget_queue_allocate(this->drawingArea); }

auto XournalWidget::initHScrolling(XournalWidget* self) -> void {
    GtkScrollable* scrollableWidget = GTK_SCROLLABLE(self->drawingArea);
    GtkAdjustment* hadjustment = gtk_scrollable_get_hadjustment(scrollableWidget);
    gtk_adjustment_configure(hadjustment, self->x, self->x - 150, self->x + 150, STEP_INCREMENT, STEP_INCREMENT, 100);
    g_signal_connect(G_OBJECT(hadjustment), "value-changed", G_CALLBACK(XournalWidget::horizontalScroll), self);
}

auto XournalWidget::initVScrolling(XournalWidget* self) -> void {
    GtkScrollable* scrollableWidget = GTK_SCROLLABLE(self->drawingArea);
    GtkAdjustment* vadjustment = gtk_scrollable_get_vadjustment(scrollableWidget);
    gtk_adjustment_configure(vadjustment, self->y, self->y - 150, self->y + 150, STEP_INCREMENT, STEP_INCREMENT, 100);
    g_signal_connect(G_OBJECT(vadjustment), "value-changed", G_CALLBACK(XournalWidget::verticalScroll), self);
}

auto XournalWidget::sizeAllocateCallback(GtkWidget* drawingArea, GdkRectangle* allocation, XournalWidget* self)
        -> void {
    Rectangle<double> documentSize = self->renderer->getDocumentSize();
    GtkScrollable* scrollableWidget = GTK_SCROLLABLE(drawingArea);
    GtkAdjustment* hadjustment = gtk_scrollable_get_hadjustment(scrollableWidget);
    GtkAdjustment* vadjustment = gtk_scrollable_get_vadjustment(scrollableWidget);

    if (self->renderer->isInfiniteVertically()) {
        gtk_adjustment_set_lower(vadjustment, -1.5 * allocation->height);
        gtk_adjustment_set_upper(vadjustment, 1.5 * allocation->height);
    } else {
        gtk_adjustment_set_lower(vadjustment, documentSize.y);
        gtk_adjustment_set_upper(vadjustment, documentSize.y + documentSize.height * self->scale);
    }
    if (self->renderer->isInfiniteHorizontally()) {
        gtk_adjustment_set_lower(hadjustment, -1.5 * allocation->width);
        gtk_adjustment_set_upper(hadjustment, 1.5 * allocation->width);
    } else {
        gtk_adjustment_set_lower(hadjustment, documentSize.x);
        gtk_adjustment_set_upper(hadjustment, documentSize.x + documentSize.width * self->scale);
    }
    gtk_adjustment_set_page_size(vadjustment, allocation->height);
    gtk_adjustment_set_page_size(hadjustment, allocation->width);
    gtk_adjustment_set_page_increment(vadjustment, allocation->height - STEP_INCREMENT);
    gtk_adjustment_set_page_increment(hadjustment, allocation->width - STEP_INCREMENT);
}

auto XournalWidget::realizeCallback(GtkWidget* drawingArea, XournalWidget* self) -> void {
    // Disable event compression
    gdk_window_set_event_compression(gtk_widget_get_window(drawingArea), false);
}

auto XournalWidget::drawCallback(GtkWidget* drawArea, cairo_t* cr, XournalWidget* self) -> gboolean {
    double x1 = NAN, x2 = NAN, y1 = NAN, y2 = NAN;
    cairo_clip_extents(cr, &x1, &y1, &x2, &y2);

    // render background
    GtkStyleContext* context = self->renderer->getGtkStyleContext();
    gtk_render_background(context, cr, x1, y1, x2 - x1, y2 - y1);

    // cairo clip is relative to viewport position
    Rectangle<double> clippingRect(self->x + x1, self->y + y1, x2 - x1, y2 - y1);

    Rectangle<double> documentSize = self->renderer->getDocumentSize();
    bool hInfinite = self->renderer->isInfiniteHorizontally();
    bool vInfinite = self->renderer->isInfiniteVertically();
    int allocWidth = gtk_widget_get_allocated_width(drawArea);
    int allocHeight = gtk_widget_get_allocated_height(drawArea);

    // if width / height of document (multiplied by scale) is smaller than widget width translate the cairo context
    if (!hInfinite && documentSize.width * self->scale < allocWidth) {
        double borderWidth = (allocWidth - documentSize.width) / 2;
        clippingRect.width = std::min(clippingRect.width, documentSize.width * self->scale);
        cairo_translate(cr, borderWidth, 0);
    }
    if (!vInfinite && documentSize.height * self->scale < allocHeight) {
        double borderHeight = (allocHeight - documentSize.height) / 2;
        clippingRect.height = std::min(clippingRect.height, documentSize.height * self->scale);
        cairo_translate(cr, 0, borderHeight);
    }

    self->renderer->render(cr, clippingRect, self->scale);
    return true;
}

auto XournalWidget::updateScrollbar(GtkAdjustment* adj, double value, bool infinite) -> void {
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
    this->queueRedraw();
}

auto XournalWidget::horizontalScroll(GtkAdjustment* hadjustment, XournalWidget* self) -> void {
    double xDiff = gtk_adjustment_get_value(hadjustment);
    self->x = xDiff * self->scale;
    self->updateScrollbar(hadjustment, xDiff, self->renderer->isInfiniteHorizontally());
}

auto XournalWidget::verticalScroll(GtkAdjustment* vadjustment, XournalWidget* self) -> void {
    double yDiff = gtk_adjustment_get_value(vadjustment);
    self->y = yDiff * self->scale;
    self->updateScrollbar(vadjustment, yDiff, self->renderer->isInfiniteVertically());
}

auto XournalWidget::setScale(double scale) -> void {
    this->scale = scale;
    this->queueAllocate();
}

auto XournalWidget::setVisibleArea(const Rectangle<double>& rect) -> void {}

auto XournalWidget::repaintVisibleArea(const Rectangle<double>& rect) -> void {}

auto XournalWidget::repaintViewport(const Rectangle<double>& rect) -> void {}

auto XournalWidget::getViewport() -> Rectangle<double> {
    return Rectangle<double>(0, 0, gtk_widget_get_allocated_width(this->drawingArea),
                             gtk_widget_get_allocated_height(this->drawingArea));
}
