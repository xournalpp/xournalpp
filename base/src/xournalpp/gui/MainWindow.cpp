#include "MainWindow.h"

#include <gtkmm.h>

#include "xournalpp/gui/widgets/XournalWidget.h"

MainWindow::MainWindow(XournalppStore store): store(std::move(store)) {
    auto cssProvider = gtk_css_provider_new();
    gtk_css_provider_load_from_resource(cssProvider, "/ui/xournalpp.css");
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(cssProvider),
                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(cssProvider);

    auto gladeBuilder = Gtk::Builder::create_from_resource("/ui/main.glade");
    gladeBuilder->get_widget("mainWindow", this->window);

    // add XournalWidget
    Gtk::Container* box = nullptr;
    gladeBuilder->get_widget("boxContents", box);
    auto scrolledWindow = Gtk::ScrolledWindow{};
    XournalWidget xournal{this->store[&AppState::settings], this->store[&AppState::viewport], this->store};
    scrolledWindow.add(xournal);
    box->add(scrolledWindow);
}

MainWindow::~MainWindow() { delete this->window; }

auto MainWindow::getGtkWindow() -> Gtk::Window* { return this->window; }
