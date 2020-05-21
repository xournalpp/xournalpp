#include "MainWindow.h"

#include <gtk-4.0/gtk/gtk.h>

MainWindow::MainWindow(GtkApplication* app, XournalppStore store): store(std::move(store)) {
    auto gladeBuilder = gtk_builder_new_from_resource("/ui/main.glade");
    this->window = GTK_WINDOW(gtk_builder_get_object(gladeBuilder, "mainWindow"));
    gtk_window_set_application(this->window, app);
}

auto MainWindow::show() -> void { gtk_window_present(this->window); }
