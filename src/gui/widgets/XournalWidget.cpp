#include "XournalWidget.h"

#include <cmath>

#include <config-debug.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>

#include "control/Control.h"
#include "control/settings/Settings.h"
#include "control/tools/EditSelection.h"
#include "gui/Layout.h"
#include "gui/Shadow.h"
#include "gui/XournalView.h"
#include "gui/inputdevices/InputContext.h"

#include "Rectangle.h"
#include "Util.h"

XournalWidget::XournalWidget(XournalView* view, std::shared_ptr<InputContext> inputContext):
        view(view), input(std::move(inputContext)), x(0), y(0), selection(nullptr) {
    this->layout = std::make_unique<Layout>(view);
    this->init();
}

auto XournalWidget::init() -> void {
    this->drawingArea = gtk_drawing_area_new();
    gtk_widget_set_hexpand(this->drawingArea, true);
    gtk_widget_set_vexpand(this->drawingArea, true);
    g_signal_connect(G_OBJECT(drawingArea), "size-allocate", G_CALLBACK(XournalWidget::sizeAllocateCallback), this);
    g_signal_connect(G_OBJECT(drawingArea), "realize", G_CALLBACK(XournalWidget::realizeCallback), this);
    g_signal_connect(G_OBJECT(drawingArea), "draw", G_CALLBACK(XournalWidget::drawCallback), this);
    if (this->input) {
        this->input->connect(this);
    }
}

XournalWidget::~XournalWidget() {
    if (selection) {
        delete selection;
    }
    gtk_widget_destroy(this->drawingArea);
};

auto XournalWidget::getGtkWidget() -> GtkWidget* { return this->drawingArea; }

auto XournalWidget::getLayout() -> Layout* { return this->layout.get(); }

auto XournalWidget::repaintArea(int x1, int y1, int x2, int y2) -> void {
    x1 -= this->x;
    x2 -= this->x;
    y1 -= this->y;
    y2 -= this->y;

    if (x2 < 0 || y2 < 0) {
        return;  // outside visible area
    }

    GtkAllocation alloc = {0};
    gtk_widget_get_allocation(this->drawingArea, &alloc);

    if (x1 > alloc.width || y1 > alloc.height) {
        return;  // outside visible area
    }

    gtk_widget_queue_draw_area(this->drawingArea, x1, y1, x2 - x1, y2 - y1);
}

auto XournalWidget::getVisibleArea(XojPageView* p) -> Rectangle* {
    GtkAdjustment* vadj = this->view->getVerticalAdjustment();
    GtkAdjustment* hadj = this->view->getHorizontalAdjustment();

    GdkRectangle r2;
    r2.x = static_cast<int>(gtk_adjustment_get_value(hadj));
    r2.y = static_cast<int>(gtk_adjustment_get_value(vadj));
    r2.width = static_cast<int>(gtk_adjustment_get_page_size(hadj));
    r2.height = static_cast<int>(gtk_adjustment_get_page_size(vadj));

    GdkRectangle r1;
    r1.x = p->getX();
    r1.y = p->getY();
    r1.width = p->getDisplayWidth();
    r1.height = p->getDisplayHeight();

    GdkRectangle r3 = {0, 0, 0, 0};
    gdk_rectangle_intersect(&r1, &r2, &r3);

    if (r3.width == 0 && r3.height == 0) {
        return nullptr;
    }

    r3.x -= r1.x;
    r3.y -= r1.y;

    double zoom = this->view->getZoom();

    if (r3.x < 0 || r3.y < 0) {
        g_warning("XournalWidget:gtk_xournal_get_visible_area: intersection rectangle coordinates are negative which "
                  "should never happen");
    }

    return new Rectangle(std::max(r3.x, 0) / zoom, std::max(r3.y, 0) / zoom, r3.width / zoom, r3.height / zoom);
}

auto XournalWidget::sizeAllocateCallback(GtkWidget* drawingArea, GdkRectangle* allocation, XournalWidget* self)
        -> void {
    if (gtk_widget_get_realized(drawingArea)) {
        gdk_window_move_resize(gtk_widget_get_window(self->drawingArea), allocation->x, allocation->y,
                               allocation->width, allocation->height);
    }
    self->layout->layoutPages(allocation->width, allocation->height);
}

auto XournalWidget::realizeCallback(GtkWidget* drawingArea, XournalWidget* self) -> void {
    // Disable event compression
    gdk_window_set_event_compression(gtk_widget_get_window(drawingArea), false);
}

auto XournalWidget::drawCallback(GtkWidget* drawArea, cairo_t* cr, XournalWidget* self) -> gboolean {
    double x1 = NAN, x2 = NAN, y1 = NAN, y2 = NAN;
    cairo_clip_extents(cr, &x1, &y1, &x2, &y2);

    // Draw background
    Settings* settings = self->view->getControl()->getSettings();
    Util::cairo_set_source_rgbi(cr, settings->getBackgroundColor());
    cairo_rectangle(cr, x1, y1, x2 - x1, y2 - y1);
    cairo_fill(cr);

    Rectangle clippingRect(x1 - 10, y1 - 10, x2 - x1 + 20, y2 - y1 + 20);

    for (auto&& pv: self->view->getViewPages()) {
        int px = pv->getX();
        int py = pv->getY();
        int pw = pv->getDisplayWidth();
        int ph = pv->getDisplayHeight();

        if (!clippingRect.intersects(pv->getRect())) {
            continue;
        }

        self->drawShadow(cr, px, py, pw, ph, pv->isSelected());

        cairo_save(cr);
        cairo_translate(cr, px, py);

        pv->paintPage(cr, nullptr);
        cairo_restore(cr);
    }

    if (self->selection) {
        double zoom = self->view->getZoom();

        Redrawable* red = self->selection->getView();
        cairo_translate(cr, red->getX(), red->getY());

        self->selection->paint(cr, zoom);
    }
    return true;
}

auto XournalWidget::drawShadow(cairo_t* cr, int left, int top, int width, int height, bool selected) -> void {
    if (selected) {
        Shadow::drawShadow(cr, left - 2, top - 2, width + 4, height + 4);

        Settings* settings = this->view->getControl()->getSettings();

        // Draw border
        Util::cairo_set_source_rgbi(cr, settings->getBorderColor());
        cairo_set_line_width(cr, 4.0);
        cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT);
        cairo_set_line_join(cr, CAIRO_LINE_JOIN_BEVEL);

        cairo_move_to(cr, left, top);
        cairo_line_to(cr, left, top + height);
        cairo_line_to(cr, left + width, top + height);
        cairo_line_to(cr, left + width, top);
        cairo_close_path(cr);

        cairo_stroke(cr);
    } else {
        Shadow::drawShadow(cr, left, top, width, height);
    }
}
