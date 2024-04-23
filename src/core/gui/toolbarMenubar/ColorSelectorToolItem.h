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

#include <string>  // for string

#include <gtk/gtk.h>  // for GtkWindget

#include "control/actions/ActionRef.h"

#include "AbstractToolItem.h"  // for AbstractToolItem

class ActionDatabase;

class ColorSelectorToolItem: public AbstractToolItem {
public:
    ColorSelectorToolItem(ActionDatabase& db);
    ColorSelectorToolItem(ColorSelectorToolItem const&) = delete;
    ColorSelectorToolItem(ColorSelectorToolItem&&) noexcept = delete;
    auto operator=(ColorSelectorToolItem const&) -> ColorSelectorToolItem& = delete;
    auto operator=(ColorSelectorToolItem&&) noexcept -> ColorSelectorToolItem& = delete;
    ~ColorSelectorToolItem() override = default;

public:
    std::string getToolDisplayName() const override;
    GtkWidget* getNewToolIcon() const override;

    xoj::util::GObjectSPtr<GdkPaintable> createPaintable(GdkSurface*) const override;

    Widgetry createItem(ToolbarSide side) override;

private:
    ActionRef gAction;  ///< Corresponds to Action::TOOL_COLOR
};
