#include "MainWindow.h"

#include <gtk-4.0/gtk/gtk.h>

MainWindow::MainWindow(GtkApplication* app, XournalppStore store): store(std::move(store)) {
    this->window = reinterpret_cast<GtkWindow*>(gtk_application_window_new(app));
    auto scrolledTop = gtk_scrolled_window_new(NULL, NULL);
    auto scrolledNested = gtk_scrolled_window_new(NULL, NULL);
    auto child = gtk_image_new_from_file("/home/julius/Bilder/mr-robot-fsociety-logo-minimalism-qs-2560x1440.jpg");
    gtk_widget_set_size_request(child, 1000, 1000);
    gtk_scrolled_window_set_child(reinterpret_cast<GtkScrolledWindow*>(scrolledNested), child);
    gtk_scrolled_window_set_child(reinterpret_cast<GtkScrolledWindow*>(scrolledTop), scrolledNested);
    gtk_widget_set_hexpand(scrolledTop, true);
    gtk_widget_set_vexpand(scrolledTop, true);
    gtk_window_set_child(this->window, scrolledTop);
}

auto MainWindow::show() -> void { gtk_window_present(this->window); }
