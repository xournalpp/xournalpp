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

class ToolbarSelectionSubmenu final: public Submenu {
public:
    ToolbarSelectionSubmenu(MainWindow* win, Settings* settings, ToolMenuHandler* toolbar);
    ~ToolbarSelectionSubmenu();

public:
    void update(ToolMenuHandler* toolbarHandler, const ToolbarData* selectedToolbar);

    void setDisabled(bool disabled) override;
    void addToMenubar(Menubar& menubar) override;

private:
    xoj::util::GObjectSPtr<GMenu> stockConfigurationsSection;
    xoj::util::GObjectSPtr<GMenu> customConfigurationsSection;
    xoj::util::GObjectSPtr<GSimpleAction> gAction;
};
