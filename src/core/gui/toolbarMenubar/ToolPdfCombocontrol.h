#pragma once

#include <string>
#include <vector>

#include "gui/GladeGui.h"

#include "ToolButton.h"


class ToolMenuHandler;

class ToolPdfCombocontrol: public ToolButton {
public:
    ToolPdfCombocontrol(ToolMenuHandler* toolMenuHandler, ActionHandler* handler, std::string id);
    virtual ~ToolPdfCombocontrol();

public:
    virtual void selected(ActionGroup group, ActionType action);

protected:
    virtual GtkToolItem* newItem();
    void addMenuitem(const std::string& text, const std::string& icon, ActionType type, ActionGroup group);

private:
    ToolMenuHandler* toolMenuHandler = nullptr;
    GtkWidget* popup = nullptr;

    GtkWidget* iconWidget = nullptr;
    GtkWidget* labelWidget = nullptr;
};
