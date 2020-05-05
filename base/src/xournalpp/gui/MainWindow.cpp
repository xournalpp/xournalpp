#include "MainWindow.h"

#include <gtk/gtk.h>

MainWindow::MainWindow(GtkApplication* app, XournalppStore store):
        store(std::move(store)),
        xournal(this->store[&AppState::settings], this->store[&AppState::viewport], this->store) {
    auto cssProvider = gtk_css_provider_new();
    gtk_css_provider_load_from_resource(cssProvider, "/ui/xournalpp.css");
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(cssProvider),
                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(cssProvider);
    auto gladeBuilder = gtk_builder_new_from_resource("/ui/main.glade");
    gtk_builder_set_application(gladeBuilder, app);
    this->window = GTK_WINDOW(gtk_builder_get_object(gladeBuilder, "mainWindow"));
    gtk_window_set_application(this->window, app);

    // add XournalWidget
    auto xournalArea = gtk_builder_get_object(gladeBuilder, "boxContents");
    auto scrolledWindow = gtk_scrolled_window_new(nullptr, nullptr);
    gtk_container_add(GTK_CONTAINER(scrolledWindow), this->xournal.getGtkWidget());
    gtk_container_add(GTK_CONTAINER(xournalArea), scrolledWindow);
}

auto MainWindow::show() -> void { gtk_window_present(this->window); }
