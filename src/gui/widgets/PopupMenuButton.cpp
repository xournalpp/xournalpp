#include "PopupMenuButton.h"

static void menu_detacher(GtkWidget* widget, GtkMenu* menu) {
    // Nothing to do
}
// See gtkmenutooltogglebutton.cpp
static void menu_position_func(GtkMenu* menu, int* x, int* y, gboolean* push_in, GtkWidget* widget) {
    GtkRequisition minimum_size;
    GtkRequisition menu_req;
    gtk_widget_get_preferred_size(GTK_WIDGET(menu), &minimum_size, &menu_req);

    GtkTextDirection direction = gtk_widget_get_direction(widget);

    GdkScreen* screen = gtk_widget_get_screen(GTK_WIDGET(menu));

    gint monitor_num = gdk_screen_get_monitor_at_window(screen, gtk_widget_get_window(widget));

    if (monitor_num < 0) {
        monitor_num = 0;
    }
    GdkRectangle monitor;
    gdk_screen_get_monitor_geometry(screen, monitor_num, &monitor);

    GtkAllocation arrow_allocation;
    gtk_widget_get_allocation(widget, &arrow_allocation);

    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);

    gdk_window_get_origin(gtk_widget_get_window(widget), x, y);
    *x += allocation.x;
    *y += allocation.y;

    if (direction == GTK_TEXT_DIR_LTR) {
        *x += std::max(allocation.width - menu_req.width, 0);
    } else if (menu_req.width > allocation.width) {
        *x -= menu_req.width - allocation.width;
    }

    if ((*y + arrow_allocation.height + menu_req.height) <= monitor.y + monitor.height) {
        *y += arrow_allocation.height;
    } else if ((*y - menu_req.height) >= monitor.y) {
        *y -= menu_req.height;
    } else if (monitor.y + monitor.height - (*y + arrow_allocation.height) > *y) {
        *y += arrow_allocation.height;
    } else {
        *y -= menu_req.height;
    }

    *push_in = false;
}

PopupMenuButton::PopupMenuButton(GtkWidget* button, GtkWidget* menu): button(button), menu(menu) {
    g_signal_connect(button, "clicked", G_CALLBACK(+[](GtkButton* button, PopupMenuButton* self) {
                         gtk_menu_popup(GTK_MENU(self->menu), nullptr, nullptr, (GtkMenuPositionFunc)menu_position_func,
                                        button, 0, gtk_get_current_event_time());

                         gtk_menu_shell_select_first(GTK_MENU_SHELL(self->menu), false);

                         // GTK 3.22: gtk_menu_popup_at_widget(menu, button, GDK_GRAVITY_SOUTH_WEST,
                         // GDK_GRAVITY_NORTH_WEST, nullptr);
                     }),
                     this);

    gtk_menu_attach_to_widget(GTK_MENU(menu), button, menu_detacher);
}

PopupMenuButton::~PopupMenuButton() = default;

void PopupMenuButton::setMenu(GtkWidget* menu) {
    gtk_menu_detach(GTK_MENU(this->menu));
    this->menu = menu;

    gtk_menu_attach_to_widget(GTK_MENU(menu), button, menu_detacher);
}
