#include "SidebarPreviewBaseEntry.h"

#include <glib-object.h>  // for G_CALLBACK, g_object_ref
#include <gtk/gtk.h>      //

#include "control/Control.h"                // for Control
#include "control/jobs/XournalScheduler.h"  // for XournalScheduler
#include "model/XojPage.h"                  // for XojPage
#include "util/safe_casts.h"                // for floor_cast

#include "SidebarPreviewBase.h"  // for SidebarPreviewBase

SidebarPreviewBaseEntry::SidebarPreviewBaseEntry(SidebarPreviewBase* sidebar, const PageRef& page):
        sidebar(sidebar), page(page), button(gtk_button_new(), xoj::util::adopt) {
    // A spinner until the miniature gets rendered for the first time.
    // The miniature will only be rendered if/when it needs to actually be displayed
    auto* spin = gtk_spinner_new();
    gtk_spinner_start(GTK_SPINNER(spin));
    gtk_button_set_child(GTK_BUTTON(button.get()), spin);
    gtk_button_set_has_frame(GTK_BUTTON(button.get()), false);
    gtk_widget_add_css_class(button.get(), "miniature");

    updateSize();

    g_signal_connect(this->button.get(), "clicked", G_CALLBACK(+[](GtkButton*, gpointer self) {
                         static_cast<SidebarPreviewBaseEntry*>(self)->mouseButtonPressCallback();
                         return true;
                     }),
                     this);

    // Set up right button clicks to pop up the context menu
    auto* ctrl = gtk_gesture_click_new();
    gtk_widget_add_controller(button.get(), GTK_EVENT_CONTROLLER(ctrl));
    gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(ctrl), GDK_BUTTON_SECONDARY);
    g_signal_connect(ctrl, "pressed",
                     G_CALLBACK(+[](GtkGestureClick* g, gint n_press, gdouble x, gdouble y, gpointer d) {
                         if (n_press == 1) {
                             auto* self = static_cast<SidebarPreviewBaseEntry*>(d);
                             self->mouseButtonPressCallback();
                             self->sidebar->openPreviewContextMenu(
                                     x, y, gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(g)));
                         }
                     }),
                     this);
}

SidebarPreviewBaseEntry::~SidebarPreviewBaseEntry() {
    this->sidebar->getControl()->getScheduler()->removeSidebar(this);
}

void SidebarPreviewBaseEntry::setSelected(bool selected) {
    if (this->selected == selected) {
        return;
    }
    this->selected = selected;
    if (selected) {
        gtk_widget_add_css_class(this->button.get(), "page-selected");
    } else {
        gtk_widget_remove_css_class(this->button.get(), "page-selected");
    }
}

void SidebarPreviewBaseEntry::repaint() { sidebar->getControl()->getScheduler()->addRepaintSidebar(this); }

void SidebarPreviewBaseEntry::updateSize() {
    this->DPIscaling = 1;  // It should really be gtk_widget_get_scale_factor(this->button.get()) but it causes a weird
                           // positionning bug when adding a new page
    // To avoid having a black line, we use floor rather than ceil
    this->imageWidth = floor_cast<int>(page->getWidth() * sidebar->getZoom());
    this->imageHeight = floor_cast<int>(page->getHeight() * sidebar->getZoom());
    gtk_widget_set_size_request(gtk_button_get_child(GTK_BUTTON(this->button.get())), imageWidth, imageHeight);
}

void SidebarPreviewBaseEntry::setVerticalPosition(Interval<int> pos) { this->verticalPosition = pos; }

auto SidebarPreviewBaseEntry::getVerticalPosition() const -> Interval<int> { return verticalPosition; }

void SidebarPreviewBaseEntry::ensureRendered() {
    if (neverRendered) {
        repaint();
        neverRendered = false;
    }
}

auto SidebarPreviewBaseEntry::getWidget() const -> GtkWidget* { return this->button.get(); }

void SidebarPreviewBaseEntry::setMiniature(xoj::util::WidgetSPtr child) {
    // This is called from the PreviewJob thread. Only call gtk functions from the main thread!
    Util::execInUiThread(
            [w = this->button, child = std::move(child)]() { gtk_button_set_child(GTK_BUTTON(w.get()), child.get()); });
}
