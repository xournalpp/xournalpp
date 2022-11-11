/*
 * Xournal++
 *
 * The Main menu bar
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>
#include <string_view>

#include <gio/gio.h>  // for GMenu, GMenuItem...

#include "util/raii/GObjectSPtr.h"

#include "config-features.h"  // for ENABLE_PLUGINS

class GladeSearchpath;
class MainWindow;

class RecentDocumentsSubmenu;
class ToolbarSelectionSubmenu;
class PageTypeSubmenu;
class PluginsSubmenu;

class Menubar {
public:
    Menubar();
    ~Menubar() noexcept;

    void populate(const GladeSearchpath* gladeSearchPath, MainWindow* win);

public:
    inline GMenuModel* getModel() const { return menu; }
    inline ToolbarSelectionSubmenu& getToolbarSelectionSubmenu() const { return *toolbarSelectionSubmenu; }
    inline PageTypeSubmenu& getPageTypeSubmenu() const { return *pageTypeSubmenu; }

    void setDisabled(bool disabled);

    template <typename gobj, typename Fun>
    gobj* get(const std::string_view& name, Fun&& converter) const {
        gobj* w = converter(gtk_builder_get_object(const_cast<GtkBuilder*>(builder.get()), name.data()));
        if (w == nullptr) {
            g_warning("Menubar::get: Could not find menu object: \"%s\"", name.data());
        }
        return w;
    }

private:
    xoj::util::GObjectSPtr<GtkBuilder> builder;
    GMenuModel* menu;

    // Dynamically created submenus -- also add to forEachSubmenu() below
    std::unique_ptr<RecentDocumentsSubmenu> recentDocumentsSubmenu;
    std::unique_ptr<ToolbarSelectionSubmenu> toolbarSelectionSubmenu;
    std::unique_ptr<PageTypeSubmenu> pageTypeSubmenu;
#ifdef ENABLE_PLUGINS
    std::unique_ptr<PluginsSubmenu> pluginsSubmenu;
#endif

    template <class Fun>
    void forEachSubmenu(Fun&& fun) {
        fun(*recentDocumentsSubmenu);
        fun(*toolbarSelectionSubmenu);
        fun(*pageTypeSubmenu);
#ifdef ENABLE_PLUGINS
        fun(*pluginsSubmenu);
#endif
    }
};
