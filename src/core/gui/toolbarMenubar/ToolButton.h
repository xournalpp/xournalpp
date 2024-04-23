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

#include <memory>
#include <string>  // for string, allocator

#include <gtk/gtk.h>  // for GtkWidget

#include "enums/Action.enum.h"  // for Action
#include "gui/PopoverFactory.h"
#include "util/raii/GVariantSPtr.h"

#include "ItemWithNamedIcon.h"  // for ItemWithNamedIcon


class ToolButton: public ItemWithNamedIcon {
public:
    ToolButton(std::string id, Category cat, Action action, std::string iconName, std::string description, bool toggle);
    ToolButton(std::string id, Category cat, Action action, GVariant* target, std::string iconName,
               std::string description);
    ~ToolButton() override = default;

public:
    std::string getToolDisplayName() const override;
    void setPopoverFactory(const PopoverFactory* factory);

protected:
    Widgetry createItem(ToolbarSide side) override;

    const char* getIconName() const override;

protected:
    std::string iconName;
    std::string description;
    /// @brief If set, a MenuButton is added with this popover
    const PopoverFactory* popoverFactory = nullptr;
    Action action;
    /// @brief If set, the action target value the button corresponds to
    xoj::util::GVariantSPtr target;
    /// @brief Whether or not the button is a ToggleButton.
    bool toggle;
};
