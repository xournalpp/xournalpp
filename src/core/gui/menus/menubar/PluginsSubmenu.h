/*
 * Xournal++
 *
 * Submenu listing plugins
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <vector>

#include <gio/gio.h>
#include <gtk/gtk.h>

#include "AbstractSubmenu.h"

class PluginController;

class PluginsSubmenu: public Submenu {
public:
    PluginsSubmenu(PluginController* pluginController, GtkApplicationWindow* win);
    virtual ~PluginsSubmenu() noexcept = default;

public:
    void setDisabled(bool disabled) override;
    void addToMenubar(MainWindow* win) override;

private:
    PluginController* pluginController;
    std::vector<GMenuModel*> sections;

private:
    static constexpr auto SUBMENU_ID = "menuitemPlugin";
};
