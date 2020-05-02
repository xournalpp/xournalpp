#include "MainWindow.h"

#include <gtk/gtk.h>

MainWindow::MainWindow(lager::reader<AppState> state, lager::context<Action> context):
        state(std::move(state)), context(std::move(context)) {
    auto cssProvider = gtk_css_provider_new();
    gtk_css_provider_load_from_resource(cssProvider, "xournalpp.css");
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(cssProvider),
                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(cssProvider);
    auto gladeBuilder = gtk_builder_new_from_resource("ui/main.glade");
    this->window = GTK_WINDOW(gtk_builder_get_object(gladeBuilder, "mainWindow"));
}

auto MainWindow::show() -> void { gtk_widget_show_all(this->window); }
