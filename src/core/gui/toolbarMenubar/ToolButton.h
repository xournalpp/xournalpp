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

#include <string>  // for string, allocator

#include <gdk-pixbuf/gdk-pixbuf.h>  // for GdkPixbuf
#include <gtk/gtk.h>                // for GtkWidget, GtkToolItem

#include "enums/ActionGroup.enum.h"  // for ActionGroup
#include "enums/ActionType.enum.h"   // for ActionType

#include "AbstractToolItem.h"  // for AbstractToolItem

class ActionHandler;

class ToolButton: public AbstractToolItem {
public:
    ToolButton(ActionHandler* handler, std::string id, ActionType type, std::string iconName, std::string description,
               GtkWidget* menuitem = nullptr);
    ToolButton(ActionHandler* handler, std::string id, ActionType type, ActionGroup group, bool toolToggleOnlyEnable,
               std::string iconName, std::string description, GtkWidget* menuitem = nullptr);

    ~ToolButton() override;

public:
    /**
     * Register a popup menu entry, create a popup menu, if none is there
     *
     * @param name The name of the item
     * @param iconName To load an icon
     * @return The created menu item
     */
    GtkWidget* registerPopupMenuEntry(const std::string& name, const std::string& iconName = "");

    void updateDescription(const std::string& description);
    std::string getToolDisplayName() const override;
    void setActive(bool active);

protected:
    GtkToolItem* newItem() override;

    GtkWidget* getNewToolIcon() const override;
    GdkPixbuf* getNewToolPixbuf() const override;

private:
    std::string iconName;
    std::string description;
};
