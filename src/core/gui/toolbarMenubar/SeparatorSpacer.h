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

#include <gtk/gtk.h>  // for GtkWidget

#include "gui/toolbarMenubar/model/ColorPalette.h"
#include "util/Color.h"       // for Color
#include "util/NamedColor.h"  // for NamedColor

#include "AbstractToolItem.h"  // for AbstractToolItem

class ActionDatabase;

template <bool spacer>
class SeparatorLikeItem: public AbstractToolItem {
public:
    SeparatorLikeItem();
    SeparatorLikeItem(SeparatorLikeItem const&) = delete;
    SeparatorLikeItem(SeparatorLikeItem&&) noexcept = delete;
    auto operator=(SeparatorLikeItem const&) -> SeparatorLikeItem& = delete;
    auto operator=(SeparatorLikeItem&&) noexcept -> SeparatorLikeItem& = delete;
    ~SeparatorLikeItem() override = default;


public:
    std::string getToolDisplayName() const override;
    GtkWidget* getNewToolIcon() const override;

    Widgetry createItem(ToolbarSide side) override;

    xoj::util::GObjectSPtr<GdkPaintable> createPaintable(GdkSurface*) const override;
};

extern template class SeparatorLikeItem<false>;
extern template class SeparatorLikeItem<true>;

using SeparatorItem = SeparatorLikeItem<false>;
using Spacer = SeparatorLikeItem<true>;
