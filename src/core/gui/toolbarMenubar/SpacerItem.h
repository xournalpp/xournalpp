/*
 * Xournal++
 *
 * A spacer
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include <gtk/gtk.h>  // for GtkWidget

#include "ItemWithNamedIcon.h"

class SpacerItem: public ItemWithNamedIcon {
public:
    SpacerItem(const char* id);
    ~SpacerItem() override = default;

public:
    const char* getIconName() const override;
    std::string getToolDisplayName() const override;

protected:
    Widgetry createItem(ToolbarSide side) override;
};
