/*
 * Xournal++
 *
 * A small toolbar whose content varies depending on the selected tool
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include <gtk/gtk.h>  // for GtkWidget

#include "AbstractToolItem.h"  // for AbstractToolItem

class ToolMenuHandler;

class AdaptativeToolkit: public AbstractToolItem {
public:
    AdaptativeToolkit(const char* id, ToolMenuHandler* handler);
    ~AdaptativeToolkit() override = default;

public:
    std::string getToolDisplayName() const override;

protected:
    xoj::util::WidgetSPtr createItem(bool horizontal) override;

    GtkWidget* getNewToolIcon() const override;

protected:
    ToolMenuHandler* handler;
};
