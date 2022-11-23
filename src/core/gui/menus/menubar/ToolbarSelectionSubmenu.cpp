#include "ToolbarSelectionSubmenu.h"

#include <algorithm>
#include <string>  // for operator==, string

#include <glib-object.h>  // for G_CALLBACK, g_sig...

#include "control/settings/Settings.h"  // for Settings
#include "gui/MainWindow.h"             // for MainWindow
#include "gui/menus/StaticAssertActionNamespace.h"
#include "gui/toolbarMenubar/ToolMenuHandler.h"     // for ToolMenuHandler
#include "gui/toolbarMenubar/model/ToolbarData.h"   // for ToolbarData
#include "gui/toolbarMenubar/model/ToolbarModel.h"  // for ToolbarModel

#include "Menubar.h"

namespace {
auto createToolbarSelectionMenuItem(const ToolbarData* toolbarData) {

    std::string action = ToolbarSelectionSubmenu::G_ACTION_NAMESPACE;
    action += ToolbarSelectionSubmenu::G_ACTION_NAME;
    action += "('";
    action += toolbarData->getId();
    action += "')";

    return xoj::util::GObjectSPtr<GMenuItem>(g_menu_item_new(toolbarData->getName().c_str(), action.c_str()),
                                             xoj::util::adopt);
}

void toolbarSelectionMenuChangeStateCallback(GSimpleAction* ga, GVariant* parameter, MainWindow* window) {
    g_simple_action_set_state(ga, parameter);

    size_t l = 0;
    const char* p = g_variant_get_string(parameter, &l);
    std::string toolbarId(p, l);

    window->toolbarSelected(toolbarId);
}
};  // namespace

ToolbarSelectionSubmenu::ToolbarSelectionSubmenu(MainWindow* win, Settings* settings, ToolMenuHandler* toolbar):
        gAction(g_simple_action_new_stateful(G_ACTION_NAME, G_VARIANT_TYPE_STRING,
                                             g_variant_new_string(settings->getSelectedToolbar().c_str())),
                xoj::util::adopt) {
    const auto& toolbars = *toolbar->getModel()->getToolbars();

    auto it = toolbars.begin();
    this->stockConfigurationsSection.reset(g_menu_new(), xoj::util::adopt);
    for (; it != toolbars.end() && (*it)->isPredefined(); ++it) {
        g_menu_append_item(this->stockConfigurationsSection.get(), createToolbarSelectionMenuItem(*it).get());
    }
    this->customConfigurationsSection.reset(g_menu_new(), xoj::util::adopt);
    for (; it != toolbars.end(); ++it) {
        g_menu_append_item(this->customConfigurationsSection.get(), createToolbarSelectionMenuItem(*it).get());
    }

    g_signal_connect(G_OBJECT(gAction.get()), "change-state", G_CALLBACK(toolbarSelectionMenuChangeStateCallback), win);
    static_assert(is_action_namespace_match<decltype(win)>(G_ACTION_NAMESPACE));
    g_action_map_add_action(G_ACTION_MAP(win->getWindow()), G_ACTION(gAction.get()));
}

ToolbarSelectionSubmenu::~ToolbarSelectionSubmenu() = default;

void ToolbarSelectionSubmenu::setDisabled(bool disabled) {
    g_simple_action_set_enabled(gAction.get(), !disabled);
    gtk_widget_set_sensitive(menuItem.get(), !disabled);
}

static void moveAndAppendAllContent(GtkWidget* from, GtkWidget* to) {
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
                gtk_menu_shell_append(data->shell, item);
                g_object_unref(item);
            },
            &data);
}

void ToolbarSelectionSubmenu::addToMenubar(MainWindow* win) {
    GtkWidget* parent = win->get("menuitemViewToolbar");
    GtkWidget* previousMenu = win->get(SUBMENU_ID);

    xoj::util::GObjectSPtr<GMenu> submenu(g_menu_new(), xoj::util::adopt);
    g_menu_append_section(submenu.get(), nullptr, G_MENU_MODEL(this->stockConfigurationsSection.get()));
    g_menu_append_section(submenu.get(), nullptr, G_MENU_MODEL(this->customConfigurationsSection.get()));

    this->menuItem.reset(gtk_menu_new_from_model(G_MENU_MODEL(submenu.get())), xoj::util::adopt);

    // Move everything that was in previousMenu to the new menu
    // The other way around does not work (for some reason, the menu entries are not connected to the actions)
    moveAndAppendAllContent(previousMenu, this->menuItem.get());

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(parent), this->menuItem.get());
}

void ToolbarSelectionSubmenu::update(ToolMenuHandler* toolbarHandler, const ToolbarData* selectedToolbar) {
    const auto& toolbars = *toolbarHandler->getModel()->getToolbars();
    // The first half stockConfigurationsSection of the menu has already been generated: fast forward to the first
    // custom config
    auto it =
            std::find_if_not(toolbars.begin(), toolbars.end(), [](const ToolbarData* d) { return d->isPredefined(); });
    this->customConfigurationsSection.reset(g_menu_new(), xoj::util::adopt);
    for (; it != toolbars.end(); ++it) {
        g_menu_append_item(this->customConfigurationsSection.get(), createToolbarSelectionMenuItem(*it).get());
    }
    // Does not fire a "change-state" signal
    g_simple_action_set_state(gAction.get(), g_variant_new_string(selectedToolbar->getId().c_str()));
}
