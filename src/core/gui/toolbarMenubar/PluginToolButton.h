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

#include <gtk/gtk.h>  // for GtkWidget, GtkToolItem

#include "ItemWithNamedIcon.h"  // for ItemWithNamedIcon

struct ToolbarButtonEntry;

class PluginToolButton: public ItemWithNamedIcon {
public:
    PluginToolButton(ToolbarButtonEntry* t);

    ~PluginToolButton() override;
    std::string getToolDisplayName() const override;

protected:
    Widgetry createItem(ToolbarSide side) override;
    GtkWidget* getNewToolIcon() const override;

    const char* getIconName() const override;

private:
    ToolbarButtonEntry* t;
};
