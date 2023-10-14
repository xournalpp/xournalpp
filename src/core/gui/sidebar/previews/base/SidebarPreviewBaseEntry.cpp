#include "SidebarPreviewBaseEntry.h"

#include <memory>  // for __shared_ptr_access

#include <gdk/gdk.h>      // for GdkEvent, GDK_BUTTON_PRESS
#include <glib-object.h>  // for G_CALLBACK, g_object_ref

#include "control/Control.h"                // for Control
#include "control/jobs/XournalScheduler.h"  // for XournalScheduler
#include "control/settings/Settings.h"      // for Settings
#include "gui/Shadow.h"                     // for Shadow
#include "model/XojPage.h"                  // for XojPage
#include "util/Color.h"                     // for cairo_set_source_rgbi
#include "util/i18n.h"                      // for _

#include "SidebarPreviewBase.h"  // for SidebarPreviewBase

SidebarPreviewBaseEntry::SidebarPreviewBaseEntry(SidebarPreviewBase* sidebar, const PageRef& page):
        sidebar(sidebar), page(page) {
    this->widget = GTK_WIDGET(g_object_ref_sink(gtk_button_new()));
    gtk_widget_show(this->widget);

    updateSize();
    gtk_widget_set_events(widget, GDK_EXPOSURE_MASK);

    g_signal_connect(this->widget, "draw", G_CALLBACK(drawCallback), this);

    g_signal_connect(this->widget, "clicked", G_CALLBACK(+[](GtkWidget* widget, SidebarPreviewBaseEntry* self) {
                         self->mouseButtonPressCallback();
                         return true;
                     }),
                     this);

    const auto clickCallback = G_CALLBACK(+[](GtkWidget* widget, GdkEvent* event, SidebarPreviewBaseEntry* self) {
        // Open context menu on right mouse click
        if (event->type == GDK_BUTTON_PRESS) {
            auto mouseEvent = reinterpret_cast<GdkEventButton*>(event);
            if (mouseEvent->button == 3) {
                self->mouseButtonPressCallback();
                self->sidebar->openPreviewContextMenu();
                return true;
            }
        }
        return false;
    });
    g_signal_connect_after(this->widget, "button-press-event", clickCallback, this);
}

SidebarPreviewBaseEntry::~SidebarPreviewBaseEntry() {
    this->sidebar->getControl()->getScheduler()->removeSidebar(this);
    this->page = nullptr;

    gtk_widget_destroy(this->widget);
    this->widget = nullptr;

    if (this->crBuffer) {
        cairo_surface_destroy(this->crBuffer);
        this->crBuffer = nullptr;
    }
}

auto SidebarPreviewBaseEntry::drawCallback(GtkWidget* widget, cairo_t* cr, SidebarPreviewBaseEntry* preview)
        -> gboolean {
    preview->paint(cr);
    return true;
}

void SidebarPreviewBaseEntry::setSelected(bool selected) {
    if (this->selected == selected) {
        return;
    }
    this->selected = selected;

    gtk_widget_queue_draw(this->widget);
}

void SidebarPreviewBaseEntry::repaint() { sidebar->getControl()->getScheduler()->addRepaintSidebar(this); }

void SidebarPreviewBaseEntry::drawLoadingPage() {
    GtkAllocation alloc;
    gtk_widget_get_allocation(widget, &alloc);

    this->crBuffer = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, alloc.width, alloc.height);

    double zoom = sidebar->getZoom();

    cairo_t* cr2 = cairo_create(this->crBuffer);
    cairo_matrix_t defaultMatrix = {0};
    cairo_get_matrix(cr2, &defaultMatrix);

    cairo_translate(cr2, Shadow::getShadowTopLeftSize() + 2, Shadow::getShadowTopLeftSize() + 2);

    cairo_scale(cr2, zoom, zoom);

    const char* txtLoading = _("Loading...");

    cairo_text_extents_t ex;
    cairo_set_source_rgb(cr2, 0.5, 0.5, 0.5);
    cairo_select_font_face(cr2, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr2, 70.0);
    cairo_text_extents(cr2, txtLoading, &ex);
    cairo_move_to(cr2, (page->getWidth() - ex.width) / 2 - ex.x_bearing,
                  (page->getHeight() - ex.height) / 2 - ex.y_bearing);
    cairo_show_text(cr2, txtLoading);

    cairo_destroy(cr2);
}

void SidebarPreviewBaseEntry::paint(cairo_t* cr) {
    bool doRepaint = false;

    this->drawingMutex.lock();

    if (this->crBuffer == nullptr) {
        drawLoadingPage();
        doRepaint = true;
    }

    cairo_set_source_surface(cr, this->crBuffer, 0, 0);
    cairo_paint(cr);

    double height = page->getHeight() * sidebar->getZoom();
    double width = page->getWidth() * sidebar->getZoom();

    if (this->selected) {
        // Draw border
        Util::cairo_set_source_rgbi(cr, sidebar->getControl()->getSettings()->getBorderColor());
        cairo_set_line_width(cr, 2);
        cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT);
        cairo_set_line_join(cr, CAIRO_LINE_JOIN_BEVEL);

        cairo_rectangle(cr, Shadow::getShadowTopLeftSize() + 0.5, Shadow::getShadowTopLeftSize() + 0.5, width + 3,
                        height + 3);

        cairo_stroke(cr);

        cairo_set_operator(cr, CAIRO_OPERATOR_ATOP);
        Shadow::drawShadow(cr, Shadow::getShadowTopLeftSize(), Shadow::getShadowTopLeftSize(), width + 4, height + 4);
    } else {
        cairo_set_operator(cr, CAIRO_OPERATOR_ATOP);
        Shadow::drawShadow(cr, Shadow::getShadowTopLeftSize() + 2, Shadow::getShadowTopLeftSize() + 2, width, height);
    }

    this->drawingMutex.unlock();

    if (doRepaint) {
        repaint();
    }
}

void SidebarPreviewBaseEntry::updateSize() {
    gtk_widget_set_size_request(this->widget, getWidgetWidth(), getWidgetHeight());
}

auto SidebarPreviewBaseEntry::getWidgetWidth() -> int {
    return page->getWidth() * sidebar->getZoom() + Shadow::getShadowBottomRightSize() + Shadow::getShadowTopLeftSize() +
           4;
}

auto SidebarPreviewBaseEntry::getWidgetHeight() -> int {
    return page->getHeight() * sidebar->getZoom() + Shadow::getShadowBottomRightSize() +
           Shadow::getShadowTopLeftSize() + 4;
}

auto SidebarPreviewBaseEntry::getWidth() -> int { return getWidgetWidth(); }

auto SidebarPreviewBaseEntry::getHeight() -> int { return getWidgetHeight(); }

auto SidebarPreviewBaseEntry::getWidget() -> GtkWidget* { return this->widget; }
