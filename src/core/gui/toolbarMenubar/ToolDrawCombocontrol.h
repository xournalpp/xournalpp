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
#include <vector>  // for vector

#include <gtk/gtk.h>  // for GtkWidget, GtkToolItem

#include "enums/ActionGroup.enum.h"  // for ActionGroup
#include "enums/ActionType.enum.h"   // for ActionType

#include "ToolButton.h"  // for ToolButton

class ToolMenuHandler;
class ToolDrawType;
class ActionHandler;

class ToolDrawCombocontrol: public ToolButton {
public:
    ToolDrawCombocontrol(ToolMenuHandler* toolMenuHandler, ActionHandler* handler, std::string id);
    ~ToolDrawCombocontrol() override;

public:
    void selected(ActionGroup group, ActionType action) override;

protected:
    GtkToolItem* newItem() override;
    void createMenuItem(const std::string& name, const std::string& icon, ActionType type);

private:
    ToolMenuHandler* toolMenuHandler = nullptr;

    GtkWidget* iconWidget = nullptr;
    GtkWidget* labelWidget = nullptr;

    std::vector<ToolDrawType*> drawTypes;
};
