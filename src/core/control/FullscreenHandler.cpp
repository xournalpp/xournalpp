#include "FullscreenHandler.h"

#include <cairo.h>        // for cairo_t
#include <glib-object.h>  // for g_object_new, g_type_register...
#include <glib.h>         // for gint, gboolean

#include "control/Control.h"
#include "control/settings/Settings.h"  // for Settings
#include "gui/MainWindow.h"
#include "util/StringUtils.h"           // for StringUtils


FullscreenHandler::FullscreenHandler(Settings* settings): settings(settings) {}

FullscreenHandler::~FullscreenHandler() = default;

auto FullscreenHandler::isFullscreen() const -> bool { return this->fullscreen; }

void FullscreenHandler::enableFullscreen(Control* ctrl) { gtk_window_fullscreen(GTK_WINDOW(ctrl->getGtkWindow())); }

void FullscreenHandler::disableFullscreen(Control* ctrl) {
    gtk_window_unfullscreen(GTK_WINDOW(ctrl->getGtkWindow()));

    for (const auto& w: hiddenFullscreenWidgets) {
        gtk_widget_show(w.get());
    }
    hiddenFullscreenWidgets.clear();

    if (this->sidebarHidden) {
        ctrl->setShowSidebar(true);
        this->sidebarHidden = false;
    }

    if (this->menubarHidden) {
        ctrl->setHideMenubar(false);
        this->menubarHidden = false;
    }
}

void FullscreenHandler::setFullscreen(Control* ctrl, bool enabled) {
    if (enabled) {
        enableFullscreen(ctrl);
    } else {
        disableFullscreen(ctrl);
    }

    this->fullscreen = enabled;
}
