#pragma once

#include <string>  // for string

#include <gtk/gtk.h>  // for GtkWidget, GtkToolItem

#include "enums/ActionGroup.enum.h"  // for ActionGroup
#include "enums/ActionType.enum.h"   // for ActionType

#include "ToolButton.h"  // for ToolButton

class ToolMenuHandler;
class ActionHandler;

class ToolPdfCombocontrol: public ToolButton {
public:
    ToolPdfCombocontrol(ToolMenuHandler* toolMenuHandler, ActionHandler* handler, std::string id);
    ~ToolPdfCombocontrol() override;

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
