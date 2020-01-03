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
#include "XournalType.h"

class ToolMenuHandler;

class ToolSelectCombocontrol: public ToolButton {
public:
    ToolSelectCombocontrol(ToolMenuHandler* toolMenuHandler, ActionHandler* handler, string id);
    virtual ~ToolSelectCombocontrol();

public:
    virtual void selected(ActionGroup group, ActionType action);

protected:
    virtual GtkToolItem* newItem();
    void addMenuitem(const string& text, const string& icon, ActionType type, ActionGroup group);

private:
    ToolMenuHandler* toolMenuHandler = nullptr;
    GtkWidget* popup = nullptr;

    GtkWidget* iconWidget = nullptr;
    GtkWidget* labelWidget = nullptr;
};
