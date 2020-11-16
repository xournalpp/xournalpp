#include "FullscreenHandler.h"

#include "gui/MainWindow.h"

#include "Control.h"
#include "StringUtils.h"


FullscreenHandler::FullscreenHandler(Settings* settings): settings(settings) {}

FullscreenHandler::~FullscreenHandler() = default;

auto FullscreenHandler::isFullscreen() const -> bool { return this->fullscreen; }

void FullscreenHandler::hideWidget(MainWindow* win, const string& widgetName) {
    if ("sidebarContents" == widgetName && settings->isSidebarVisible()) {
        this->sidebarHidden = true;
        win->setSidebarVisible(false);

        return;
    }

    if ("mainMenubar" == widgetName) {
        GtkWidget* mainMenubar = win->get("mainMenubar");

        if (mainMenubar == nullptr || !gtk_widget_is_visible(mainMenubar)) {
            // Menu not visible (global menu or something like this)
            return;
        }
        gtk_widget_hide(mainMenubar);
        menubarHidden = true;
        return;
    }
}

void FullscreenHandler::enableFullscreen(MainWindow* win) {
    gtk_window_fullscreen(static_cast<GtkWindow*>(*win));

    string hideWidgets = settings->getFullscreenHideElements();
    for (const string& s: StringUtils::split(hideWidgets, ',')) {
        hideWidget(win, s);
    }
}

void FullscreenHandler::disableFullscreen(MainWindow* win) {
    gtk_window_unfullscreen(static_cast<GtkWindow*>(*win));

    for (GtkWidget* w: hiddenFullscreenWidgets) {
        gtk_widget_show(w);
    }
    hiddenFullscreenWidgets.clear();

    if (this->sidebarHidden) {
        this->sidebarHidden = false;
        win->setSidebarVisible(true);
    }

    if (this->menubarHidden) {
        GtkWidget* mainMenubar = win->get("mainMenubar");
        if(mainMenubar != nullptr) {
            gtk_widget_show(mainMenubar);
        }
        menubarHidden = false;
    }
}

void FullscreenHandler::setFullscreen(MainWindow* win, bool enabled) {
    if (enabled) {
        enableFullscreen(win);
    } else {
        disableFullscreen(win);
    }

    this->fullscreen = enabled;
}

///////////////////////////////////////////////////////////////////////////////
// Widget which allow to hide the menu, but let the shortcuts active
///////////////////////////////////////////////////////////////////////////////

struct GtkInvisibleMenuClass {
    GtkFixedClass parent_class;
};

struct GtkInvisibleMenu {
    GtkFixed widget;
};

static void gtk_invisible_menu_get_preferred_width(GtkWidget* /*widget*/, gint* minimum, gint* natural) {
    *minimum = 0;
    *natural = 0;
}

static void gtk_invisible_menu_get_preferred_height(GtkWidget* /*widget*/, gint* minimum, gint* natural) {
    *minimum = 0;
    *natural = 0;
}

static auto gtk_invisible_menu_draw(GtkWidget* /*widget*/, cairo_t * /*cr*/) -> gboolean { return false; }

static void gtk_invisible_menu_class_init(GtkInvisibleMenuClass* klass) {
    auto widget_class = reinterpret_cast<GtkWidgetClass*>(klass);
    widget_class->get_preferred_width = gtk_invisible_menu_get_preferred_width;
    widget_class->get_preferred_height = gtk_invisible_menu_get_preferred_height;
    widget_class->draw = gtk_invisible_menu_draw;
}

auto gtk_invisible_get_type() -> GType {
    static GType gtk_invisible_menu_type = 0;

    if (!gtk_invisible_menu_type) {
        static const GTypeInfo gtk_inivisible_menu_info = {
                sizeof(GtkInvisibleMenuClass),
                // base initialize
                nullptr,
                // base finalize
                nullptr,
                // class initialize
                reinterpret_cast<GClassInitFunc>(gtk_invisible_menu_class_init),
                // class finalize
                nullptr,
                // class data,
                nullptr,
                // instance size
                sizeof(GtkInvisibleMenu),
                // n_preallocs
                0,
                // instance init
                nullptr,
                // value table
                nullptr};

        gtk_invisible_menu_type = g_type_register_static(GTK_TYPE_FIXED, "GtkInvisibleMenu", &gtk_inivisible_menu_info,
                                                         static_cast<GTypeFlags>(0));
    }

    return gtk_invisible_menu_type;
}

auto gtk_invisible_new() -> GtkWidget* { return GTK_WIDGET(g_object_new(gtk_invisible_get_type(), nullptr)); }
