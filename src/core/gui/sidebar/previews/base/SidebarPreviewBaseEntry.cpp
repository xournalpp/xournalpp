#include "SidebarPreviewBaseEntry.h"

#include <gdk/gdk.h>      // for GdkEvent, GDK_BUTTON_PRESS
#include <glib-object.h>  // for G_CALLBACK, g_object_ref
#include <gtk/gtk.h>      //

#include "control/Control.h"                // for Control
#include "control/jobs/XournalScheduler.h"  // for XournalScheduler
#include "control/settings/Settings.h"      // for Settings
#include "gui/Shadow.h"                     // for Shadow
#include "model/XojPage.h"                  // for XojPage
#include "util/Color.h"                     // for cairo_set_source_rgbi
#include "util/gtk4_helper.h"               //
#include "util/i18n.h"                      // for _
#include "util/safe_casts.h"                // for floor_cast

#include "SidebarPreviewBase.h"  // for SidebarPreviewBase

SidebarPreviewBaseEntry::SidebarPreviewBaseEntry(SidebarPreviewBase* sidebar, const PageRef& page):
        sidebar(sidebar), page(page), button(gtk_button_new(), xoj::util::adopt) {

    updateSize();
    // gtk_widget_set_events(this->button.get(), GDK_EXPOSURE_MASK);

    // g_signal_connect(this->button.get(), "draw", G_CALLBACK(drawCallback), this);

    g_signal_connect(this->button.get(), "clicked", G_CALLBACK(+[](GtkButton*, gpointer self) {
                         static_cast<SidebarPreviewBaseEntry*>(self)->mouseButtonPressCallback();
                         return true;
                     }),
                     this);

    // const auto clickCallback = G_CALLBACK(+[](GtkWidget*, GdkEvent* event, SidebarPreviewBaseEntry* self) {
    //     // Open context menu on right mouse click
    //     if (event->type == GDK_BUTTON_PRESS) {
    //         auto mouseEvent = reinterpret_cast<GdkEventButton*>(event);
    //         if (mouseEvent->button == 3) {
    //             self->mouseButtonPressCallback();
    //             self->sidebar->openPreviewContextMenu(event);
    //             return true;
    //         }
    //     }
    //     return false;
    // });
    // g_signal_connect_after(this->button.get(), "button-press-event", clickCallback, this);
}

SidebarPreviewBaseEntry::~SidebarPreviewBaseEntry() {
    this->sidebar->getControl()->getScheduler()->removeSidebar(this);
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

    gtk_widget_queue_draw(this->button.get());
}

void SidebarPreviewBaseEntry::repaint() { sidebar->getControl()->getScheduler()->addRepaintSidebar(this); }

void SidebarPreviewBaseEntry::drawLoadingPage() {
    this->buffer.reset(cairo_image_surface_create(CAIRO_FORMAT_ARGB32, imageWidth, imageHeight), xoj::util::adopt);

    double zoom = sidebar->getZoom();

    cairo_t* cr2 = cairo_create(this->buffer.get());
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

    if (!this->buffer) {
        drawLoadingPage();
        doRepaint = true;
    }

    cairo_set_source_surface(cr, this->buffer.get(), 0, 0);
    cairo_paint(cr);

    this->drawingMutex.unlock();

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
        Shadow::drawShadow(cr, Shadow::getShadowTopLeftSize(), Shadow::getShadowTopLeftSize(),
                           round_cast<int>(width) + 4, round_cast<int>(height) + 4);
    } else {
        cairo_set_operator(cr, CAIRO_OPERATOR_ATOP);
        Shadow::drawShadow(cr, Shadow::getShadowTopLeftSize() + 2, Shadow::getShadowTopLeftSize() + 2,
                           round_cast<int>(width), round_cast<int>(height));
    }

    const auto& recolorParams = sidebar->getControl()->getSettings()->getRecolorParameters();
    auto recolor = recolorParams.recolorizeSidebarMiniatures ? std::make_optional(recolorParams.recolor) : std::nullopt;

    if (recolor) {
        // encapsulate in save/restore to limit the scope of the clip operation
        xoj::util::CairoSaveGuard const saveGuard(cr);
        cairo_rectangle(cr, Shadow::getShadowTopLeftSize() + 2, Shadow::getShadowTopLeftSize() + 2, width, height);
        // constrain the area which is painted on
        cairo_clip(cr);
        recolor->recolorCurrentCairoRegion(cr);
    }

    if (doRepaint) {
        repaint();
    }
}

void SidebarPreviewBaseEntry::updateSize() {
    this->DPIscaling = gtk_widget_get_scale_factor(this->button.get());

    const int shadowPadding = Shadow::getShadowBottomRightSize() + Shadow::getShadowTopLeftSize() + 4;
    // To avoid having a black line, we use floor rather than ceil
    this->imageWidth = floor_cast<int>(page->getWidth() * sidebar->getZoom()) + shadowPadding;
    this->imageHeight = floor_cast<int>(page->getHeight() * sidebar->getZoom()) + shadowPadding;
    gtk_widget_set_size_request(this->button.get(), imageWidth, imageHeight);
}

auto SidebarPreviewBaseEntry::getWidget() const -> GtkWidget* { return this->button.get(); }

auto SidebarPreviewBaseEntry::getWidth() const -> int { return imageWidth; }

auto SidebarPreviewBaseEntry::getHeight() const -> int { return imageHeight; }
