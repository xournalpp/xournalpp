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

class MainWindow;

class RecentDocumentsSubmenu;
class ToolbarSelectionSubmenu;
class PluginsSubmenu;

class Menubar {
public:
    Menubar();
    ~Menubar() noexcept;

    void populate(MainWindow* win);

public:
    inline ToolbarSelectionSubmenu& getToolbarSelectionSubmenu() const { return *toolbarSelectionSubmenu; }

    void setDisabled(bool disabled);

private:
    // Dynamically created submenus -- also add to forEachSubmenu() below
    std::unique_ptr<RecentDocumentsSubmenu> recentDocumentsSubmenu;
    std::unique_ptr<ToolbarSelectionSubmenu> toolbarSelectionSubmenu;
    std::unique_ptr<PluginsSubmenu> pluginsSubmenu;

    template <class Fun>
    void forEachSubmenu(Fun&& fun) {
        fun(*recentDocumentsSubmenu);
        fun(*toolbarSelectionSubmenu);
        fun(*pluginsSubmenu);
    }
};
