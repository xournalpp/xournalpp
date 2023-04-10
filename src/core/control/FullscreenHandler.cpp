#include "FullscreenHandler.h"

#include <cairo.h>        // for cairo_t
#include <glib-object.h>  // for g_object_new, g_type_register...
#include <glib.h>         // for gint, gboolean

#include "control/settings/Settings.h"  // for Settings
#include "gui/MainWindow.h"             // for MainWindow
#include "util/StringUtils.h"           // for StringUtils

using std::string;


FullscreenHandler::FullscreenHandler(Settings* settings): settings(settings) {}

FullscreenHandler::~FullscreenHandler() = default;

auto FullscreenHandler::isFullscreen() const -> bool { return this->fullscreen; }

void FullscreenHandler::enableFullscreen(MainWindow* win) { gtk_window_fullscreen(GTK_WINDOW(win->getWindow())); }

void FullscreenHandler::disableFullscreen(MainWindow* win) {
    gtk_window_unfullscreen(GTK_WINDOW(win->getWindow()));

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
        GtkWidget* mainBox = win->get("mainBox");

        GtkWidget* parent = gtk_widget_get_parent(mainMenubar);

        // Remove menu from parent
        gtk_container_remove(GTK_CONTAINER(parent), mainMenubar);
        gtk_box_pack_start(GTK_BOX(mainBox), mainMenubar, false, true, 0);


        GValue value = G_VALUE_INIT;
        g_value_init(&value, G_TYPE_INT);
        g_value_set_int(&value, 0);
        gtk_container_child_set_property(GTK_CONTAINER(mainBox), mainMenubar, "position", &value);

        // not needed, will be recreated next time
        gtk_widget_destroy(parent);

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

static auto gtk_invisible_menu_draw(GtkWidget* /*widget*/, cairo_t* /*cr*/) -> gboolean { return false; }

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
