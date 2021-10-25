#include "FullscreenHandler.h"

#include "gui/MainWindow.h"
#include "util/StringUtils.h"

#include "Control.h"

using std::string;


FullscreenHandler::FullscreenHandler(Settings* settings): settings(settings) {}

FullscreenHandler::~FullscreenHandler() = default;

auto FullscreenHandler::isFullscreen() const -> bool { return this->fullscreen; }

// Todo (gtk4, fabian): this looks not good;
void FullscreenHandler::hideWidget(MainWindow* win, const std::string& widgetName) {
    if ("sidebarContents" == widgetName && settings->isSidebarVisible()) {
        this->sidebarHidden = true;
        win->setSidebarVisible(false);

        return;
    }

    if ("mainMenubar" == widgetName) {
        // If the menu is hidden, shortcuts are not working anymore
        // therefore the menu is not hidden, it's displayed, but invisible
        // this costs 1px at the bottom, even if the preferred size is 0px,
        // 1px is used by GTK

        GtkWidget* mainMenubar = win->get("mainMenubar");
        GtkWidget* mainBox = win->get("mainBox");

        if (mainMenubar == nullptr || !gtk_widget_is_visible(mainMenubar)) {
            // Menu not visible (global menu or something like this)
            return;
        }

        // Remove menu from parent
        gtk_box_remove(GTK_BOX(mainBox), mainMenubar);
        menubarHidden = true;

        return;
    }
}

void FullscreenHandler::enableFullscreen(MainWindow* win) {
    gtk_window_fullscreen(static_cast<GtkWindow*>(*win));

    string hideWidgets = settings->getFullscreenHideElements();
    for (const string& s: StringUtils::split(hideWidgets, ',')) { hideWidget(win, s); }
}

void FullscreenHandler::disableFullscreen(MainWindow* win) {
    gtk_window_unfullscreen(static_cast<GtkWindow*>(*win));

    for (GtkWidget* w: hiddenFullscreenWidgets) { gtk_widget_show(w); }
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

        gtk_box_prepend(GTK_BOX(mainBox), mainMenubar);
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
// Todo (gtk4, fabian): remove, there is a better way...
///////////////////////////////////////////////////////////////////////////////

struct GtkInvisibleMenuClass {
    GtkFixedClass parent_class;
};

struct GtkInvisibleMenu {
    GtkFixed widget;
};

static void gtk_invisible_menu_measure(GtkWidget* widget, GtkOrientation orientation, int for_size, int* minimum,
                                       int* natural, int* minimum_baseline, int* natural_baseline) {
    *minimum = 0;
    *natural = 0;
    *natural_baseline = 0;
    *minimum_baseline = 0;
}

static auto gtk_invisible_menu_snapshot(GtkWidget* widget, GtkSnapshot* snapshot) {}

static void gtk_invisible_menu_class_init(GtkInvisibleMenuClass* klass) {
    auto widget_class = reinterpret_cast<GtkWidgetClass*>(klass);
    widget_class->measure = gtk_invisible_menu_measure;
    widget_class->snapshot = gtk_invisible_menu_snapshot;
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
