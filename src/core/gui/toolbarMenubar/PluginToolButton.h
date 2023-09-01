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

#include "plugin/Plugin.h"

#include "AbstractToolItem.h"  // for AbstractToolItem

class PluginToolButton: public AbstractToolItem {
public:
    PluginToolButton(ToolbarButtonEntry* t);

    ~PluginToolButton() override;
    std::string getToolDisplayName() const override;

protected:
    GtkToolItem* createItem(bool horizontal) override;
    GtkToolItem* newItem() override;
    GtkWidget* getNewToolIcon() const override;

private:
    ToolbarButtonEntry* t;
};
