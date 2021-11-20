#include "PopupMenuButton.h"

// See gtkmenutooltogglebutton.cpp
static void menu_position_func(GtkWidget* menu, int* x, int* y, gboolean* push_in, GtkWidget* widget) {
    GtkRequisition minimum_size;
    GtkRequisition menu_req;
    gtk_widget_get_preferred_size(GTK_WIDGET(menu), &minimum_size, &menu_req);

    GtkTextDirection direction = gtk_widget_get_direction(widget);

    auto* display = gtk_widget_get_display(GTK_WIDGET(menu));
    GdkMonitor* monitor =
            gdk_display_get_monitor_at_surface(display, gtk_native_get_surface(gtk_widget_get_native(widget)));
    GdkRectangle monitor_rect;
    gdk_monitor_get_geometry(monitor, &monitor_rect);

    GtkAllocation arrow_allocation;
    gtk_widget_get_allocation(widget, &arrow_allocation);

    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    double dx;
    double dy;
    gtk_native_get_surface_transform(gtk_widget_get_native(widget), &dx, &dy);
    *x = dx;
    *y = dy;

    *x += allocation.x;
    *y += allocation.y;

    if (direction == GTK_TEXT_DIR_LTR) {
        *x += std::max(allocation.width - menu_req.width, 0);
    } else if (menu_req.width > allocation.width) {
        *x -= menu_req.width - allocation.width;
    }

    if ((*y + arrow_allocation.height + menu_req.height) <= monitor_rect.y + monitor_rect.height) {
        *y += arrow_allocation.height;
    } else if ((*y - menu_req.height) >= monitor_rect.y) {
        *y -= menu_req.height;
    } else if (monitor_rect.y + monitor_rect.height - (*y + arrow_allocation.height) > *y) {
        *y += arrow_allocation.height;
    } else {
        *y -= menu_req.height;
    }

    *push_in = false;
}

PopupMenuButton::PopupMenuButton(GtkWidget* button, GtkWidget* menu): button(button), menu(menu) {
    g_signal_connect(button, "clicked", G_CALLBACK(+[](GtkButton* button, PopupMenuButton* self) {
                         gtk_popover_popup(GTK_POPOVER(self->menu));
                     }),
                     this);

    gtk_box_append(GTK_BOX(menu), button);
}

PopupMenuButton::~PopupMenuButton() = default;

void PopupMenuButton::setMenu(GtkWidget* menu) {
    gtk_box_remove(GTK_BOX(menu), button);
    this->menu = menu;
    gtk_box_append(GTK_BOX(menu), button);
}
