#include "MainWindow.h"

#include <gtk/gtk.h>

MainWindow::MainWindow(XournalppStore store): store(std::move(store)) {
    auto cssProvider = gtk_css_provider_new();
    gtk_css_provider_load_from_resource(cssProvider, "/ui/xournalpp.css");
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(cssProvider),
                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(cssProvider);
    auto gladeBuilder = gtk_builder_new_from_resource("/ui/main.glade");
    this->window = GTK_WINDOW(gtk_builder_get_object(gladeBuilder, "mainWindow"));
}

auto MainWindow::show() -> void { gtk_window_present(this->window); }
