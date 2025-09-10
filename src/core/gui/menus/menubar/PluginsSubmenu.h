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

class PluginsSubmenu final: public Submenu {
public:
    PluginsSubmenu(PluginController* pluginController, GtkApplicationWindow* win);
    ~PluginsSubmenu() noexcept = default;

public:
    void setDisabled(bool disabled) override;
    void addToMenubar(Menubar& menubar) override;

private:
    xoj::util::GObjectSPtr<GMenu> submenu;
};
