#include "PluginsSubmenu.h"

#include "gui/MainWindow.h"
#include "plugin/PluginController.h"

PluginsSubmenu::PluginsSubmenu(PluginController* pluginController, GtkApplicationWindow* win) {
    sections = pluginController->createMenuSections(win);
}

void PluginsSubmenu::setDisabled(bool disabled) {
    if (this->menuItem) {
        gtk_widget_set_sensitive(this->menuItem.get(), !disabled);
    }
}

static void moveAndPrependAllContent(GtkWidget* from, GtkWidget* to) {
    gtk_menu_shell_prepend(GTK_MENU_SHELL(to), gtk_separator_menu_item_new());
    struct Data {
        GtkContainer* c;
        GtkMenuShell* shell;
    } data = {GTK_CONTAINER(from), GTK_MENU_SHELL(to)};
    gtk_container_foreach(
            data.c,
            +[](GtkWidget* item, gpointer d) {
                Data* data = reinterpret_cast<Data*>(d);
                g_object_ref(item);
                gtk_container_remove(data->c, item);
                gtk_menu_shell_prepend(data->shell, item);
                g_object_unref(item);
            },
            &data);
    gtk_widget_show_all(to);
}

void PluginsSubmenu::addToMenubar(MainWindow* win) {
    if (sections.empty()) {
        return;
    }
    GtkWidget* parent = win->get("menuitemPlugin");
    GtkWidget* previousMenu = win->get("menuPlugin");

    xoj::util::GObjectSPtr<GMenu> submenu(g_menu_new(), xoj::util::adopt);
    for (auto* section: sections) {
        g_menu_append_section(submenu.get(), nullptr, section);
    }

    this->menuItem.reset(gtk_menu_new_from_model(G_MENU_MODEL(submenu.get())), xoj::util::adopt);
    // Move everything that was in previousMenu to the new menu
    // The other way around does not work (for some reason, the menu entries are not connected to the actions)
    moveAndPrependAllContent(previousMenu, this->menuItem.get());

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(parent), this->menuItem.get());
}
