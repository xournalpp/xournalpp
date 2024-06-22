#include "ItemWithNamedIcon.h"

#include <utility>  // for move

#include "util/raii/GVariantSPtr.h"


ItemWithNamedIcon::ItemWithNamedIcon(std::string id, Category cat): AbstractToolItem(std::move(id), cat) {}

auto ItemWithNamedIcon::getNewToolIcon() const -> GtkWidget* { return gtk_image_new_from_icon_name(getIconName()); }

auto ItemWithNamedIcon::createPaintable(GdkSurface* target) const -> xoj::util::GObjectSPtr<GdkPaintable> {
    GtkIconTheme* theme = gtk_icon_theme_get_for_display(gdk_surface_get_display(target));
    return xoj::util::GObjectSPtr<GdkPaintable>(
            GDK_PAINTABLE(gtk_icon_theme_lookup_icon(theme, getIconName(), nullptr, 24,
                                                     gdk_surface_get_scale_factor(target), gtk_get_locale_direction(),
                                                     GtkIconLookupFlags(0))),
            xoj::util::adopt);
}
