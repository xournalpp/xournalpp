#include "IconNameHelper.h"

#include <gtk/gtk.h>  // for gtk_icon_theme_get_default

#include "control/settings/Settings.h"  // for Settings

IconNameHelper::IconNameHelper(Settings* settings): settings(settings) {}

auto IconNameHelper::iconName(const char* icon) const -> std::string {
    std::string xoppName = std::string("xopp-") + icon;
    auto iconName = this->settings->areStockIconsUsed() && gtk_icon_theme_has_icon(gtk_icon_theme_get_default(), icon) ?
                            std::string(icon) :
                            xoppName;
    return iconName;
}
