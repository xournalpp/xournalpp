#include "SeparatorSpacer.h"

#include "icon/ToolbarSeparatorImage.h"
#include "util/i18n.h"

template <bool spacer>
SeparatorLikeItem<spacer>::SeparatorLikeItem():
        AbstractToolItem(spacer ? "SPACER" : "SEPARATOR", AbstractToolItem::Category::SEPARATORS) {}

template <bool spacer>
auto SeparatorLikeItem<spacer>::createItem(ToolbarSide side) -> Widgetry {
    if constexpr (spacer) {
        GtkWidget* it;
        if (to_Orientation(side) == GTK_ORIENTATION_HORIZONTAL) {
            it = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
            gtk_widget_set_hexpand(it, true);
        } else {
            it = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
            gtk_widget_set_vexpand(it, true);
        }
        gtk_widget_add_css_class(it, "spacer");
        return {xoj::util::WidgetSPtr(it, xoj::util::adopt), nullptr};
    } else {
        auto* it = gtk_separator_new(to_Orientation(side) == GTK_ORIENTATION_HORIZONTAL ? GTK_ORIENTATION_VERTICAL :
                                                                                          GTK_ORIENTATION_HORIZONTAL);
        auto* proxy = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
        gtk_widget_add_css_class(proxy, "model");
        return {xoj::util::WidgetSPtr(it, xoj::util::adopt), xoj::util::WidgetSPtr(proxy, xoj::util::adopt)};
    }
}

template <bool spacer>
auto SeparatorLikeItem<spacer>::getToolDisplayName() const -> std::string {
    return spacer ? _("Spacer") : _("Separator");
}

template <bool spacer>
auto SeparatorLikeItem<spacer>::getNewToolIcon() const -> GtkWidget* {
    return ToolbarSeparatorImage::newImage(spacer ? SeparatorType::SPACER : SeparatorType::SEPARATOR);
}

template <bool spacer>
auto SeparatorLikeItem<spacer>::createPaintable(GdkSurface*) const -> xoj::util::GObjectSPtr<GdkPaintable> {
    return ToolbarSeparatorImage::newGdkPaintable(spacer ? SeparatorType::SPACER : SeparatorType::SEPARATOR);
}


template class SeparatorLikeItem<true>;
template class SeparatorLikeItem<false>;
