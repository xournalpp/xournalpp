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

#include <functional>
#include <string>  // for string, allocator

#include <gtk/gtk.h>  // for GtkWidget

#include "enums/Action.enum.h"  // for Action
#include "gui/PopoverFactory.h"
#include "util/raii/GVariantSPtr.h"

#include "ToolButton.h"

/// ToolButton with dynamic tooltip description (overrides the GtkWidget::query-tooltip signal handler)
class TooltipToolButton: public ToolButton {
public:
    TooltipToolButton(std::string id, Action action, std::string iconName, std::string description,
                      std::function<std::string()> fetchTooltip);
    ~TooltipToolButton() override = default;

    xoj::util::WidgetSPtr createItem(bool horizontal) override;

private:
    std::function<std::string()> fetchTooltip;
};
