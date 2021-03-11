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

#include <string>
#include <vector>

#include "gui/GladeGui.h"

#include "ToolButton.h"


class ToolMenuHandler;
class ToolDrawType;

class ToolDrawCombocontrol: public ToolButton {
public:
    ToolDrawCombocontrol(ToolMenuHandler* toolMenuHandler, ActionHandler* handler, std::string id);
    virtual ~ToolDrawCombocontrol();

public:
    virtual void selected(ActionGroup group, ActionType action);

protected:
    virtual GtkToolItem* newItem();
    void createMenuItem(const std::string& name, const std::string& icon, ActionType type);

private:
    ToolMenuHandler* toolMenuHandler = nullptr;

    GtkWidget* iconWidget = nullptr;
    GtkWidget* labelWidget = nullptr;

    std::vector<ToolDrawType*> drawTypes;
};
