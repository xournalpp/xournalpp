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

#include <gtk/gtk.h>  // for GtkWidget

#include "enums/Action.enum.h"  // for Action
#include "util/raii/GVariantSPtr.h"

#include "AbstractToolItem.h"  // for AbstractToolItem

class ToolButton: public AbstractToolItem {
public:
    ToolButton(std::string id, Action action, std::string iconName, std::string description, bool toggle);
    ToolButton(std::string id, Action action, GVariant* target, std::string iconName, std::string description);

    ~ToolButton() override;

public:
    void updateDescription(const std::string& description);
    std::string getToolDisplayName() const override;
    void setPopover(GtkWidget* popover);

protected:
    GtkToolItem* createItem(bool horizontal) override;
    GtkToolItem* newItem() override;

    GtkWidget* getNewToolIcon() const override;

protected:
    std::string iconName;
    std::string description;
    /// @brief If set, a MenuButton is added with this popover
    xoj::util::WidgetSPtr popover;
    Action newAction;
    /// @brief If set, the action target value the button corresponds to
    xoj::util::GVariantSPtr target;
    /// @brief Whether or not the button is a ToggleButton.
    bool toggle;
};
