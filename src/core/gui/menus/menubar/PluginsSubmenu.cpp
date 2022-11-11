#include "PluginsSubmenu.h"

#include "plugin/PluginController.h"
#include "util/Assert.h"
#include "util/i18n.h"

#include "Menubar.h"

/// position of the plugin submenu, from the end of the menu bar: 1 means just before the last one (About)
static constexpr int POSITION_IN_MENUBAR_FROM_END = 1;

PluginsSubmenu::PluginsSubmenu(PluginController* pluginController, GtkApplicationWindow* win):
        submenu(g_menu_new(), xoj::util::adopt) {
    xoj::util::GObjectSPtr<GMenu> firstSection(g_menu_new(), xoj::util::adopt);
    g_menu_append(firstSection.get(), _("Plugin Manager"), "win.plugin-manager");

    g_menu_append_section(submenu.get(), nullptr, G_MENU_MODEL(firstSection.get()));

    for (auto* section: pluginController->createMenuSections(win)) {
        g_menu_append_section(submenu.get(), nullptr, section);
    }
}

void PluginsSubmenu::setDisabled(bool disabled) {
    // TODO!!
}

void PluginsSubmenu::addToMenubar(Menubar& menubar) {
    GMenuModel* mainmenu = menubar.getModel();
    int insertionPlace = g_menu_model_get_n_items(mainmenu) - POSITION_IN_MENUBAR_FROM_END;
    xoj_assert(insertionPlace >= 0);
    g_menu_insert_submenu(G_MENU(mainmenu), insertionPlace, _("Plugin"), G_MENU_MODEL(submenu.get()));
}
