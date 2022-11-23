/*
 * Xournal++
 *
 * Submenu for toolbar selection
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "util/raii/GObjectSPtr.h"

#include "AbstractSubmenu.h"

class Settings;
class ToolMenuHandler;
class ToolbarData;
class MainWindow;

class ToolbarSelectionSubmenu: public Submenu {
public:
    ToolbarSelectionSubmenu(MainWindow* win, Settings* settings, ToolMenuHandler* toolbar);
    virtual ~ToolbarSelectionSubmenu();

public:
    void update(ToolMenuHandler* toolbarHandler, const ToolbarData* selectedToolbar);

    void setDisabled(bool disabled) override;
    void addToMenubar(MainWindow* win) override;

private:
    xoj::util::GObjectSPtr<GMenu> stockConfigurationsSection;
    xoj::util::GObjectSPtr<GMenu> customConfigurationsSection;
    xoj::util::GObjectSPtr<GSimpleAction> gAction;

public:
    static constexpr auto G_ACTION_NAMESPACE = "win.";
    static constexpr auto G_ACTION_NAME = "select-toolbar";
    /// id from ui/mainmenubar.xml
    static constexpr auto SUBMENU_ID = "menuViewToolbar";
};
