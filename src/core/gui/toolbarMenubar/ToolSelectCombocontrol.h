/*
 * Xournal++
 *
 * Part of the customizable toolbars
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>  // for string

#include <gtk/gtk.h>  // for GtkWidget, GtkToolItem

#include "enums/ActionGroup.enum.h"  // for ActionGroup
#include "enums/ActionType.enum.h"   // for ActionType

#include "ToolButton.h"  // for ToolButton

class ToolMenuHandler;
class ActionHandler;

class ToolSelectCombocontrol: public ToolButton {
public:
    ToolSelectCombocontrol(ToolMenuHandler* toolMenuHandler, ActionHandler* handler, std::string id);
    ~ToolSelectCombocontrol() override;

public:
    void selected(ActionGroup group, ActionType action) override;

protected:
    GtkToolItem* newItem() override;
    void addMenuitem(const std::string& text, const std::string& icon, ActionType type, ActionGroup group);

private:
    ToolMenuHandler* toolMenuHandler = nullptr;
    GtkWidget* popup = nullptr;

    GtkWidget* iconWidget = nullptr;
    GtkWidget* labelWidget = nullptr;
};
