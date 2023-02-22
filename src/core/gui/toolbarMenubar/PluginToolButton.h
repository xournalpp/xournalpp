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

#include "ToolButton.h"  // for AbstractToolItem

class PluginToolButton: public ToolButton {
public:
    PluginToolButton(ActionHandler* handler, ToolbarButtonEntry* t);

    ~PluginToolButton() override;

protected:
    GtkToolItem* createItem(bool horizontal) override;
    ToolbarButtonEntry* t;
};
