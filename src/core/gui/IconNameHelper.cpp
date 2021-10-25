#include "IconNameHelper.h"

#include "control/settings/Settings.h"

IconNameHelper::IconNameHelper(Settings* settings): settings(settings) {}

// Todo review the whole Icon Themes impl: https://wiki.gnome.org/HowDoI/ThemedIcons
auto IconNameHelper::iconName(const char* icon) const -> std::string {
    std::string xoppName = std::string("xopp-") + icon;
    auto iconName =
            this->settings->areStockIconsUsed() &&
                            gtk_icon_theme_has_icon(gtk_icon_theme_get_for_display(gdk_display_get_default()), icon) ?
                    std::string(icon) :
                    xoppName;
    return iconName;
}
