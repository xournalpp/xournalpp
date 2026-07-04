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

#include "AbstractToolItem.h"  // for AbstractToolItem

class SpacerItem: public AbstractToolItem {
public:
    SpacerItem(const char* id);
    ~SpacerItem() override = default;

public:
    std::string getToolDisplayName() const override;

protected:
    xoj::util::WidgetSPtr createItem(bool horizontal) override;

    GtkWidget* getNewToolIcon() const override;
};
