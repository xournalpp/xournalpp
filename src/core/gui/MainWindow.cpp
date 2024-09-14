#include "MainWindow.h"

#include <regex>

#include <gdk-pixbuf/gdk-pixbuf.h>  // for gdk_pixbuf_new_fr...
#include <gdk/gdk.h>                // for gdk_screen_get_de...
#include <gio/gio.h>                // for g_cancellable_is_...
#include <gtk/gtkcssprovider.h>     // for gtk_css_provider_...

#include "control/AudioController.h"                    // for AudioController
#include "control/Control.h"                            // for Control
#include "control/DeviceListHelper.h"                   // for getSourceMapping
#include "control/ScrollHandler.h"                      // for ScrollHandler
#include "control/jobs/XournalScheduler.h"              // for XournalScheduler
#include "control/layer/LayerController.h"              // for LayerController
#include "control/settings/Settings.h"                  // for Settings
#include "control/settings/SettingsEnums.h"             // for SCROLLBAR_HIDE_HO...
#include "control/zoom/ZoomControl.h"                   // for ZoomControl
#include "enums/ActionType.enum.h"                      // for ACTION_DELETE_LAYER
#include "gui/FloatingToolbox.h"                        // for FloatingToolbox
#include "gui/GladeGui.h"                               // for GladeGui
#include "gui/PdfFloatingToolbox.h"                     // for PdfFloatingToolbox
#include "gui/SearchBar.h"                              // for SearchBar
#include "gui/inputdevices/InputEvents.h"               // for INPUT_DEVICE_TOUC...
#include "gui/menus/menubar/Menubar.h"                  // for Menubar
#include "gui/menus/menubar/ToolbarSelectionSubmenu.h"  // for ToolbarSelectionSubmenu
#include "gui/scroll/ScrollHandling.h"                  // for ScrollHandling
#include "gui/toolbarMenubar/ToolMenuHandler.h"         // for ToolMenuHandler
#include "gui/toolbarMenubar/model/ToolbarData.h"       // for ToolbarData
#include "gui/toolbarMenubar/model/ToolbarModel.h"      // for ToolbarModel
#include "gui/widgets/SpinPageAdapter.h"                // for SpinPageAdapter
#include "gui/widgets/XournalWidget.h"                  // for gtk_xournal_get_l...
#include "util/GListView.h"                             // for GListView, GListV...
#include "util/PathUtil.h"                              // for getConfigFile
#include "util/Util.h"                                  // for execInUiThread, npos
#include "util/XojMsgBox.h"                             // for XojMsgBox
#include "util/glib_casts.h"                            // for wrap_for_once_v
#include "util/i18n.h"                                  // for FS, _F
#include "util/raii/CStringWrapper.h"                   // for OwnedCString

#include "GladeSearchpath.h"     // for GladeSearchpath
#include "ToolbarDefinitions.h"  // for TOOLBAR_DEFINITIO...
#include "XournalView.h"         // for XournalView
#include "config-dev.h"          // for TOOLBAR_CONFIG
#include "filesystem.h"          // for path, exists

#ifdef __APPLE__
// the following header file contains a definition of struct Point that conflicts with model/Point.h
#define Point Point_CF
#include <CoreFoundation/CoreFoundation.h>
#undef Point
#endif

using std::string;


static void themeCallback(GObject*, GParamSpec*, gpointer data) { static_cast<MainWindow*>(data)->updateColorscheme(); }

MainWindow::MainWindow(GladeSearchpath* gladeSearchPath, Control* control, GtkApplication* parent):
        GladeGui(gladeSearchPath, "main.glade", "mainWindow"),
        control(control),
        toolbar(std::make_unique<ToolMenuHandler>(control, this)),
        menubar(std::make_unique<Menubar>()) {

    gtk_window_set_application(GTK_WINDOW(getWindow()), parent);

    toolbar->populate(gladeSearchPath);
    menubar->populate(this);

    panedContainerWidget.reset(get("panelMainContents"), xoj::util::ref);
    boxContainerWidget.reset(get("mainContentContainer"), xoj::util::ref);
    mainContentWidget.reset(get("boxContents"), xoj::util::ref);
    sidebarWidget.reset(get("sidebar"), xoj::util::ref);

    loadMainCSS(gladeSearchPath, "xournalpp.css");

    GtkOverlay* overlay = GTK_OVERLAY(get("mainOverlay"));
    this->pdfFloatingToolBox = std::make_unique<PdfFloatingToolbox>(this, overlay);
    this->floatingToolbox = std::make_unique<FloatingToolbox>(this, overlay);

    for (int i = 0; i < TOOLBAR_DEFINITIONS_LEN; i++) {
        this->toolbarWidgets[i].reset(get(TOOLBAR_DEFINITIONS[i].guiName), xoj::util::ref);
    }

    initXournalWidget();

    setSidebarVisible(control->getSettings()->isSidebarVisible());

    // Window handler
    g_signal_connect(this->window, "delete-event", xoj::util::wrap_for_g_callback_v<deleteEventCallback>,
                     this->control);
#if GTK_MAJOR_VERSION == 3
    g_signal_connect(this->window, "notify::is-maximized", xoj::util::wrap_for_g_callback_v<windowMaximizedCallback>,
                     this);
#else
    g_signal_connect(this->window, "notify::maximized", xoj::util::wrap_for_g_callback_v<windowMaximizedCallback>,
                     this);
#endif

    g_signal_connect(get("buttonCloseSidebar"), "clicked", xoj::util::wrap_for_g_callback_v<buttonCloseSidebarClicked>,
                     this);

    // "watch over" all key events
    auto keyPropagate = +[](GtkWidget* w, GdkEvent* e, gpointer) {
        return gtk_window_propagate_key_event(GTK_WINDOW(w), (GdkEventKey*)(e));
    };
    g_signal_connect(this->window, "key-press-event", G_CALLBACK(keyPropagate), nullptr);
    g_signal_connect(this->window, "key-release-event", G_CALLBACK(keyPropagate), nullptr);

    // need to create tool buttons registered in plugins, so they can be added to toolbars
    control->registerPluginToolButtons(this->toolbar.get());

    createToolbar();

    setToolbarVisible(control->getSettings()->isToolbarVisible());

    GtkWidget* menuViewSidebarVisible = get("menuViewSidebarVisible");
    g_signal_connect(menuViewSidebarVisible, "toggled", G_CALLBACK(viewShowSidebar), this);

    GtkWidget* menuViewToolbarsVisible = get("menuViewToolbarsVisible");
    g_signal_connect(menuViewToolbarsVisible, "toggled", G_CALLBACK(viewShowToolbar), this);

    updateScrollbarSidebarPosition();

    gtk_window_set_default_size(GTK_WINDOW(this->window), control->getSettings()->getMainWndWidth(),
                                control->getSettings()->getMainWndHeight());

    if (control->getSettings()->isMainWndMaximized()) {
        gtk_window_maximize(GTK_WINDOW(this->window));
    } else {
        gtk_window_unmaximize(GTK_WINDOW(this->window));
    }

    getSpinPageNo()->addListener(this->control->getScrollHandler());


    Util::execInUiThread([=]() {
        // Execute after the window is visible, else the check won't work
        initHideMenu();
    });

    // Drag and Drop
    g_signal_connect(this->window, "drag-data-received", G_CALLBACK(dragDataRecived), this);

    gtk_drag_dest_set(this->window, GTK_DEST_DEFAULT_ALL, nullptr, 0, GDK_ACTION_COPY);
    gtk_drag_dest_add_uri_targets(this->window);
    gtk_drag_dest_add_image_targets(this->window);
    gtk_drag_dest_add_text_targets(this->window);

    LayerCtrlListener::registerListener(control->getLayerController());

    g_signal_connect(gtk_widget_get_settings(this->window), "notify::gtk-theme-name", G_CALLBACK(themeCallback), this);
    g_signal_connect(gtk_widget_get_settings(this->window), "notify::gtk-application-prefer-dark-theme",
                     G_CALLBACK(themeCallback), this);

    updateColorscheme();
}

gboolean MainWindow::isKeyForClosure(GtkAccelKey* key, GClosure* closure, gpointer data) { return closure == data; }

gboolean MainWindow::invokeMenu(GtkWidget* widget) {
    // g_warning("invoke_menu %s", gtk_widget_get_name(widget));
    gtk_widget_activate(widget);
    return TRUE;
}

void MainWindow::rebindAcceleratorsMenuItem(GtkWidget* widget, gpointer user_data) {
    if (GTK_IS_MENU_ITEM(widget)) {
        GtkAccelGroup* newAccelGroup = reinterpret_cast<GtkAccelGroup*>(user_data);
        GList* menuAccelClosures = gtk_widget_list_accel_closures(widget);
        for (GClosure& closure: GListView<GClosure>(menuAccelClosures)) {
            GtkAccelGroup* accelGroup = gtk_accel_group_from_accel_closure(&closure);
            GtkAccelKey* key = gtk_accel_group_find(accelGroup, isKeyForClosure, &closure);
            gtk_accel_group_connect(newAccelGroup, key->accel_key, key->accel_mods, GtkAccelFlags(0),
                                    g_cclosure_new_swap(G_CALLBACK(MainWindow::invokeMenu), widget, nullptr));
        }
        g_list_free(menuAccelClosures);
        MainWindow::rebindAcceleratorsSubMenu(widget, newAccelGroup);
    }
}

void MainWindow::rebindAcceleratorsSubMenu(GtkWidget* widget, gpointer user_data) {
    if (GTK_IS_MENU_ITEM(widget)) {
        GtkMenuItem* menuItem = reinterpret_cast<GtkMenuItem*>(widget);
        GtkWidget* subMenu = gtk_menu_item_get_submenu(menuItem);
        if (GTK_IS_CONTAINER(subMenu)) {
            gtk_container_foreach(reinterpret_cast<GtkContainer*>(subMenu), rebindAcceleratorsMenuItem, user_data);
        }
    }
}

// When the Menubar is hidden, accelerators no longer work so rebind them to the MainWindow
// It should be called after all plugins have been initialised so that their injected menu items are captured
void MainWindow::rebindMenubarAccelerators() {
    this->globalAccelGroup = gtk_accel_group_new();
    gtk_window_add_accel_group(GTK_WINDOW(this->getWindow()), this->globalAccelGroup);

    GtkMenuBar* menuBar = (GtkMenuBar*)this->get("mainMenubar");
    gtk_container_foreach(reinterpret_cast<GtkContainer*>(menuBar), rebindAcceleratorsSubMenu, this->globalAccelGroup);
}

MainWindow::~MainWindow() = default;

/**
 * Topmost widgets, to check if there is a menu above
 */
const char* TOP_WIDGETS[] = {"tbTop1", "tbTop2", "mainContainerBox", nullptr};


void MainWindow::toggleMenuBar(MainWindow* win) {
    GtkWidget* menu = win->get("mainMenubar");
    if (gtk_widget_is_visible(menu)) {
        gtk_widget_hide(menu);
    } else {
        gtk_widget_show(menu);
    }
}

struct ThemeProperties {
    bool dark;
    bool darkSuffix;
    std::string rootname;  ///< Name without any putative -dark suffix
};
static ThemeProperties getThemeProperties(GtkWidget* w) {
    xoj::util::OwnedCString name;
    [[maybe_unused]] bool useEnv = false;
    // Gtk prioritizes GTK_THEME over GtkSettings content
    // cf https://gitlab.gnome.org/GNOME/gtk/blob/90d84a2af8b367bd5a5312b3fa3b67563462c0ef/gtk/gtksettings.c#L1567-L1622
    if (auto* p = g_getenv("GTK_THEME")) {
        *(name.contentReplacer()) = g_strdup(p);
        useEnv = true;
    } else {
        g_object_get(gtk_widget_get_settings(w), "gtk-theme-name", name.contentReplacer(), nullptr);
    }

    // Try to figure out if the theme is dark or light
    // Some themes handle their dark variant via "gtk-application-prefer-dark-theme" while other just append "-dark"
    const std::regex nameparser("([a-zA-Z-]+?)([:-][dD]ark)?");
    std::cmatch sm;
    std::regex_match(name.get(), sm, nameparser);

    ThemeProperties props;
    if (sm.size() < 3) {
        g_warning("Fails to extract theme root name from: \"%s\"", name.get());
        props.rootname = name.get();
        props.darkSuffix = false;
    } else {
        props.rootname = sm[1];
        props.darkSuffix = sm[2].length() > 0;
    }
    gboolean dark = false;

#ifdef __APPLE__
    if (!useEnv) {
        CFStringRef interfaceStyle =
                (CFStringRef)CFPreferencesCopyAppValue(CFSTR("AppleInterfaceStyle"), kCFPreferencesCurrentApplication);
        if (interfaceStyle) {
            char buffer[128];
            if (CFStringGetCString(interfaceStyle, buffer, sizeof(buffer), kCFStringEncodingUTF8)) {
                std::string style = buffer;
                if (auto pos = style.find("Dark"); pos != std::string::npos) {
                    dark = true;
                }
            }
        }
    }
#else
    g_object_get(gtk_widget_get_settings(w), "gtk-application-prefer-dark-theme", &dark, nullptr);
#endif

    g_debug("Extracted theme info: Name = %s, rootname = %s, dark = %s", name.get(), props.rootname.c_str(),
            dark ? "true" : "false");

    props.dark = props.darkSuffix || dark;  // Some themes handle their dark variant via this setting

    return props;
}

void MainWindow::updateColorscheme() {
    g_signal_handlers_block_by_func(gtk_widget_get_settings(this->window),
                                    reinterpret_cast<gpointer>(G_CALLBACK(themeCallback)), this);
    auto variant = control->getSettings()->getThemeVariant();
    if (variant == THEME_VARIANT_USE_SYSTEM) {
        gtk_settings_reset_property(gtk_widget_get_settings(this->window), "gtk-application-prefer-dark-theme");
        if (modifiedGtkSettingsTheme) {
            // Some bug in Gtk makes an infinite loop despite us blocking the signals
            gtk_settings_reset_property(gtk_widget_get_settings(this->window), "gtk-theme-name");
            modifiedGtkSettingsTheme = false;
        }
    }
    auto props = getThemeProperties(this->window);

    this->darkMode = (props.dark && variant != THEME_VARIANT_FORCE_LIGHT) || variant == THEME_VARIANT_FORCE_DARK;

    // Set up icons
    {
        const auto uiPath = this->getGladeSearchPath()->getFirstSearchPath();
        const auto lightColorIcons = (uiPath / "iconsColor-light").u8string();
        const auto darkColorIcons = (uiPath / "iconsColor-dark").u8string();
        const auto lightLucideIcons = (uiPath / "iconsLucide-light").u8string();
        const auto darkLucideIcons = (uiPath / "iconsLucide-dark").u8string();

        // icon load order from lowest priority to highest priority
        std::vector<std::string> iconLoadOrder = {};
        const auto chosenTheme = control->getSettings()->getIconTheme();
        switch (chosenTheme) {
            case ICON_THEME_COLOR:
                iconLoadOrder = {darkLucideIcons, lightLucideIcons, darkColorIcons, lightColorIcons};
                break;
            case ICON_THEME_LUCIDE:
                iconLoadOrder = {darkColorIcons, lightColorIcons, darkLucideIcons, lightLucideIcons};
                break;
            default:
                g_message("Unknown icon theme!");
        }

        if (this->darkMode) {
            for (size_t i = 0; 2 * i + 1 < iconLoadOrder.size(); ++i) {
                std::swap(iconLoadOrder[2 * i], iconLoadOrder[2 * i + 1]);
            }
        }

        for (auto& p: iconLoadOrder) {
            gtk_icon_theme_prepend_search_path(gtk_icon_theme_get_default(), p.c_str());
        }
    }

    GtkStyleContext* context = gtk_widget_get_style_context(GTK_WIDGET(this->window));

    if (this->darkMode) {
        gtk_style_context_add_class(context, "darkMode");
        g_object_set(gtk_widget_get_settings(this->window), "gtk-application-prefer-dark-theme", true, nullptr);
    } else {
        gtk_style_context_remove_class(context, "darkMode");
        g_object_set(gtk_widget_get_settings(this->window), "gtk-application-prefer-dark-theme", false, nullptr);
        if (props.darkSuffix) {  // The active theme is all dark. Remove the trailing "-dark"
            g_object_set(gtk_widget_get_settings(this->window), "gtk-theme-name", props.rootname.c_str(), nullptr);
            modifiedGtkSettingsTheme = true;
        }
    }

    {
        gchar* name = nullptr;
        g_object_get(gtk_widget_get_settings(this->window), "gtk-theme-name", &name, nullptr);
        g_debug("Theme name: %s", name);
        g_debug("Modified in GtkSettings: %s", modifiedGtkSettingsTheme ? "true" : "false");
        g_free(name);
        gboolean gtkdark = true;
        g_object_get(gtk_widget_get_settings(this->window), "gtk-application-prefer-dark-theme", &gtkdark, nullptr);
        g_debug("Theme variant: %s", gtkdark ? "dark" : "light");
        g_debug("Icon theme: %s", iconThemeToString(control->getSettings()->getIconTheme()));
    }
    g_signal_handlers_unblock_by_func(gtk_widget_get_settings(this->window), reinterpret_cast<gpointer>(themeCallback),
                                      this);
}

void MainWindow::initXournalWidget() {
    GtkWidget* boxContents = get("boxContents");


    winXournal = gtk_scrolled_window_new(nullptr, nullptr);

    setGtkTouchscreenScrollingForDeviceMapping();

    gtk_container_add(GTK_CONTAINER(boxContents), winXournal);

    GtkWidget* vpXournal = gtk_viewport_new(nullptr, nullptr);

    gtk_container_add(GTK_CONTAINER(winXournal), vpXournal);

    scrollHandling = std::make_unique<ScrollHandling>(GTK_SCROLLABLE(vpXournal));

    this->xournal = std::make_unique<XournalView>(vpXournal, control, scrollHandling.get());

    control->getZoomControl()->initZoomHandler(this->window, winXournal, xournal.get(), control);
    gtk_widget_show_all(winXournal);

    Layout* layout = gtk_xournal_get_layout(this->xournal->getWidget());
    scrollHandling->init(this->xournal->getWidget(), layout);
}

void MainWindow::setGtkTouchscreenScrollingForDeviceMapping() {
    InputDeviceClass touchscreenClass =
            DeviceListHelper::getSourceMapping(GDK_SOURCE_TOUCHSCREEN, this->getControl()->getSettings());

    setGtkTouchscreenScrollingEnabled(touchscreenClass == INPUT_DEVICE_TOUCHSCREEN &&
                                      !control->getSettings()->getTouchDrawingEnabled());
}

void MainWindow::setGtkTouchscreenScrollingEnabled(bool enabled) {
    if (!control->getSettings()->getGtkTouchInertialScrollingEnabled()) {
        enabled = false;
    }

    if (enabled == gtkTouchscreenScrollingEnabled.load() || winXournal == nullptr) {
        return;
    }

    gtkTouchscreenScrollingEnabled.store(enabled);

    Util::execInUiThread(
            [=]() {
                const bool touchScrollEnabled = gtkTouchscreenScrollingEnabled.load();

                gtk_scrolled_window_set_kinetic_scrolling(GTK_SCROLLED_WINDOW(winXournal), touchScrollEnabled);
            },
            G_PRIORITY_HIGH);
}

/**
 * Allow to hide menubar, but only if global menu is not enabled
 */
void MainWindow::initHideMenu() {
    int top = -1;
    for (int i = 0; TOP_WIDGETS[i]; i++) {
        GtkWidget* w = get(TOP_WIDGETS[i]);
        GtkAllocation allocation;
        gtk_widget_get_allocation(w, &allocation);
        if (allocation.y != -1) {
            top = allocation.y;
            break;
        }
    }

    GtkWidget* menuItem = get("menuHideMenu");
    if (top < 5) {
        // There is no menu to hide, the menu is in the globalmenu!
        gtk_widget_hide(menuItem);
    } else {
        // Menu found, allow to hide it
        g_signal_connect(menuItem, "activate",
                         G_CALLBACK(+[](GtkMenuItem* menuitem, MainWindow* self) { toggleMenuBar(self); }), this);
    }

    // Hide menubar at startup if specified in settings
    Settings* settings = control->getSettings();
    if (settings && !settings->isMenubarVisible()) {
        toggleMenuBar(this);
    }
}

auto MainWindow::getLayout() const -> Layout* { return gtk_xournal_get_layout(this->xournal->getWidget()); }

auto MainWindow::getNegativeXournalWidgetPos() const -> utl::Point<double> {
    return Util::toWidgetCoords(this->winXournal, utl::Point{0.0, 0.0});
}

auto cancellable_cancel(GCancellable* cancel) -> bool {
    g_cancellable_cancel(cancel);

    g_warning("Timeout... Cancel loading URL");

    return false;
}

void MainWindow::dragDataRecived(GtkWidget* widget, GdkDragContext* dragContext, gint x, gint y, GtkSelectionData* data,
                                 guint info, guint time, MainWindow* win) {
    GtkWidget* source = gtk_drag_get_source_widget(dragContext);
    if (source && widget == gtk_widget_get_toplevel(source)) {
        gtk_drag_finish(dragContext, false, false, time);
        return;
    }

    guchar* text = gtk_selection_data_get_text(data);
    if (text) {
        win->control->clipboardPasteText(reinterpret_cast<const char*>(text));

        g_free(text);
        gtk_drag_finish(dragContext, true, false, time);
        return;
    }

    xoj::util::GObjectSPtr<GdkPixbuf> image(gtk_selection_data_get_pixbuf(data), xoj::util::adopt);
    if (image) {
        win->control->clipboardPasteImage(image.get());

        gtk_drag_finish(dragContext, true, false, time);
        return;
    }

    gchar** uris = gtk_selection_data_get_uris(data);
    if (uris) {
        for (int i = 0; uris[i] != nullptr && i < 3; i++) {
            const char* uri = uris[i];

            GCancellable* cancel = g_cancellable_new();
            auto cancelTimeout = g_timeout_add(3000, xoj::util::wrap_for_once_v<cancellable_cancel>, cancel);

            xoj::util::GObjectSPtr<GFile> file(g_file_new_for_uri(uri), xoj::util::adopt);
            GError* err = nullptr;
            GFileInputStream* in = g_file_read(file.get(), cancel, &err);
            if (g_cancellable_is_cancelled(cancel)) {
                continue;
            }

            if (err == nullptr) {
                xoj::util::GObjectSPtr<GdkPixbuf> pixbuf(
                        gdk_pixbuf_new_from_stream(G_INPUT_STREAM(in), cancel, nullptr), xoj::util::adopt);
                if (g_cancellable_is_cancelled(cancel)) {
                    continue;
                }
                g_input_stream_close(G_INPUT_STREAM(in), cancel, nullptr);
                if (g_cancellable_is_cancelled(cancel)) {
                    continue;
                }

                if (pixbuf) {
                    win->control->clipboardPasteImage(pixbuf.get());
                }
            } else {
                g_error_free(err);
            }

            if (!g_cancellable_is_cancelled(cancel)) {
                g_source_remove(cancelTimeout);
            }
            g_object_unref(cancel);
        }

        gtk_drag_finish(dragContext, true, false, time);

        g_strfreev(uris);
    }

    gtk_drag_finish(dragContext, false, false, time);
}

void MainWindow::viewShowSidebar(GtkCheckMenuItem* checkmenuitem, MainWindow* win) {
    bool a = gtk_check_menu_item_get_active(checkmenuitem);
    if (win->control->getSettings()->isSidebarVisible() == a) {
        return;
    }
    win->setSidebarVisible(a);
}

void MainWindow::viewShowToolbar(GtkCheckMenuItem* checkmenuitem, MainWindow* win) {
    bool showToolbar = gtk_check_menu_item_get_active(checkmenuitem);
    if (win->control->getSettings()->isToolbarVisible() == showToolbar) {
        return;
    }
    win->setToolbarVisible(showToolbar);
}

auto MainWindow::getControl() const -> Control* { return control; }

void MainWindow::updateScrollbarSidebarPosition() {
    GtkWidget* panelMainContents = get("panelMainContents");

    if (winXournal != nullptr) {
        GtkScrolledWindow* scrolledWindow = GTK_SCROLLED_WINDOW(winXournal);

        ScrollbarHideType type = this->getControl()->getSettings()->getScrollbarHideType();

        bool scrollbarOnLeft = control->getSettings()->isScrollbarOnLeft();
        if (scrollbarOnLeft) {
            gtk_scrolled_window_set_placement(scrolledWindow, GTK_CORNER_TOP_RIGHT);
        } else {
            gtk_scrolled_window_set_placement(scrolledWindow, GTK_CORNER_TOP_LEFT);
        }

        gtk_widget_set_visible(gtk_scrolled_window_get_hscrollbar(scrolledWindow), !(type & SCROLLBAR_HIDE_HORIZONTAL));
        gtk_widget_set_visible(gtk_scrolled_window_get_vscrollbar(scrolledWindow), !(type & SCROLLBAR_HIDE_VERTICAL));

        gtk_scrolled_window_set_overlay_scrolling(scrolledWindow,
                                                  !control->getSettings()->isScrollbarFadeoutDisabled());
    }

    // If the sidebar isn't visible, we can't change its position!
    if (!this->sidebarVisible) {
        return;
    }

    GtkWidget* sidebar = get("sidebar");
    GtkWidget* boxContents = get("boxContents");

    int divider = gtk_paned_get_position(GTK_PANED(panelMainContents));
    bool sidebarRight = control->getSettings()->isSidebarOnRight();
    if (sidebarRight == (gtk_paned_get_child2(GTK_PANED(panelMainContents)) == sidebar)) {
        // Already correct
        return;
    }


    GtkAllocation allocation;
    gtk_widget_get_allocation(panelMainContents, &allocation);
    divider = allocation.width - divider;


    g_object_ref(sidebar);
    g_object_ref(boxContents);

    gtk_container_remove(GTK_CONTAINER(panelMainContents), sidebar);
    gtk_container_remove(GTK_CONTAINER(panelMainContents), boxContents);

    if (sidebarRight) {
        gtk_paned_pack1(GTK_PANED(panelMainContents), boxContents, true, false);
        gtk_paned_pack2(GTK_PANED(panelMainContents), sidebar, false, false);
    } else {
        gtk_paned_pack1(GTK_PANED(panelMainContents), sidebar, false, false);
        gtk_paned_pack2(GTK_PANED(panelMainContents), boxContents, true, false);
    }

    gtk_paned_set_position(GTK_PANED(panelMainContents), divider);
    g_object_unref(sidebar);
    g_object_unref(boxContents);
}

void MainWindow::buttonCloseSidebarClicked(GtkButton* button, MainWindow* win) { win->setSidebarVisible(false); }

auto MainWindow::deleteEventCallback(GtkWidget* widget, GdkEvent* event, Control* control) -> bool {
    control->quit();

    return true;
}

void MainWindow::setMenubarVisible(bool visible) {
    GtkWidget* menu = get("mainMenubar");
    if (visible && !gtk_widget_is_visible(menu)) {
        toggleMenuBar(this);
    } else if (!visible && gtk_widget_is_visible(menu)) {
        toggleMenuBar(this);
    }
}

void MainWindow::setSidebarVisible(bool visible) {
    Settings* settings = control->getSettings();

    settings->setSidebarVisible(visible);
    if (!visible && (control->getSidebar() != nullptr)) {
        saveSidebarSize();
    }

    if (visible != this->sidebarVisible) {
        // Due to a GTK bug, we can't just hide the sidebar widget in the GtkPaned.
        // If we do this, we create a dead region where the pane separator was previously.
        // In this region, we can't use the touchscreen to start horizontal strokes.
        // As such:
        if (!visible) {
            gtk_container_remove(GTK_CONTAINER(panedContainerWidget.get()), mainContentWidget.get());
            gtk_container_remove(GTK_CONTAINER(boxContainerWidget.get()), panedContainerWidget.get());
            gtk_container_add(GTK_CONTAINER(boxContainerWidget.get()), mainContentWidget.get());
            this->sidebarVisible = false;
        } else {
            gtk_container_remove(GTK_CONTAINER(boxContainerWidget.get()), mainContentWidget.get());
            gtk_container_add(GTK_CONTAINER(panedContainerWidget.get()), mainContentWidget.get());
            gtk_container_add(GTK_CONTAINER(boxContainerWidget.get()), panedContainerWidget.get());
            this->sidebarVisible = true;

            updateScrollbarSidebarPosition();
        }
    }

    gtk_widget_set_visible(sidebarWidget.get(), visible);

    if (visible) {
        gtk_paned_set_position(GTK_PANED(panedContainerWidget.get()), settings->getSidebarWidth());
    }

    GtkWidget* w = get("menuViewSidebarVisible");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(w), visible);
}

void MainWindow::setToolbarVisible(bool visible) {
    Settings* settings = control->getSettings();

    settings->setToolbarVisible(visible);
    for (auto& w: this->toolbarWidgets) {
        if (!visible || (gtk_toolbar_get_n_items(GTK_TOOLBAR(w.get())) != 0)) {
            gtk_widget_set_visible(w.get(), visible);
        }
    }

    GtkWidget* w = get("menuViewToolbarsVisible");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(w), visible);
}

void MainWindow::saveSidebarSize() {
    this->control->getSettings()->setSidebarWidth(gtk_paned_get_position(GTK_PANED(panedContainerWidget.get())));
}

void MainWindow::setMaximized(bool maximized) { this->maximized = maximized; }

auto MainWindow::isMaximized() const -> bool { return this->maximized; }

auto MainWindow::isDarkTheme() const -> bool { return this->darkMode; }

auto MainWindow::getXournal() const -> XournalView* { return xournal.get(); }

auto MainWindow::windowMaximizedCallback(GObject* window, GParamSpec*, MainWindow* win) -> void {
    win->setMaximized(gtk_window_is_maximized(GTK_WINDOW(window)));
}

void MainWindow::toolbarSelected(const std::string& id) {
    const auto& toolbars = *toolbar->getModel()->getToolbars();
    auto it = std::find_if(toolbars.begin(), toolbars.end(), [&](const ToolbarData* d) { return d->getId() == id; });
    toolbarSelected(it == toolbars.end() ? nullptr : *it);
}

void MainWindow::toolbarSelected(ToolbarData* d) {
    if (!d || this->selectedToolbar == d) {
        return;
    }

    Settings* settings = control->getSettings();
    settings->setSelectedToolbar(d->getId());

    this->clearToolbar();
    this->loadToolbar(d);
}

auto MainWindow::clearToolbar() -> ToolbarData* {
    if (this->selectedToolbar != nullptr) {
        for (int i = 0; i < TOOLBAR_DEFINITIONS_LEN; i++) {
            ToolMenuHandler::unloadToolbar(this->toolbarWidgets[i].get());
        }

        this->toolbar->freeDynamicToolbarItems();
    }

    ToolbarData* oldData = this->selectedToolbar;

    this->selectedToolbar = nullptr;

    return oldData;
}

void MainWindow::loadToolbar(ToolbarData* d) {
    this->selectedToolbar = d;

    for (int i = 0; i < TOOLBAR_DEFINITIONS_LEN; i++) {
        this->toolbar->load(d, this->toolbarWidgets[i].get(), TOOLBAR_DEFINITIONS[i].propName,
                            TOOLBAR_DEFINITIONS[i].horizontal);
    }

    this->floatingToolbox->flagRecalculateSizeRequired();
}

auto MainWindow::getSelectedToolbar() const -> ToolbarData* { return this->selectedToolbar; }

auto MainWindow::getToolbarWidgets() const -> const ToolbarWidgetArray& { return toolbarWidgets; }

auto MainWindow::getToolbarName(GtkToolbar* toolbar) const -> const char* {
    for (int i = 0; i < TOOLBAR_DEFINITIONS_LEN; i++) {
        if (static_cast<void*>(this->toolbarWidgets[i].get()) == static_cast<void*>(toolbar)) {
            return TOOLBAR_DEFINITIONS[i].propName;
        }
    }

    return "";
}

void MainWindow::setControlTmpDisabled(bool disabled) {
    menubar->setDisabled(disabled);
    toolbar->setTmpDisabled(disabled);
}

void MainWindow::updateToolbarMenu() {
    menubar->getToolbarSelectionSubmenu().update(toolbar.get(), this->selectedToolbar);
}

void MainWindow::createToolbar() {
    toolbarSelected(control->getSettings()->getSelectedToolbar());

    if (auto* audioController = this->control->getAudioController(); audioController && !audioController->isPlaying()) {
        this->getToolMenuHandler()->disableAudioPlaybackButtons();
    }

    this->control->getScheduler()->unblockRerenderZoom();
}

void MainWindow::setFontButtonFont(const XojFont& font) { toolbar->setFontButtonFont(font); }

auto MainWindow::getFontButtonFont() const -> XojFont { return toolbar->getFontButtonFont(); }

void MainWindow::updatePageNumbers(size_t page, size_t pagecount, size_t pdfpage) {
    SpinPageAdapter* spinPageNo = getSpinPageNo();

    size_t min = 0;
    size_t max = pagecount;

    if (pagecount == 0) {
        min = 0;
        page = 0;
    } else {
        min = 1;
        page++;
    }

    spinPageNo->setMinMaxPage(min, max);
    spinPageNo->setPage(page);

    if (pdfpage != npos) {
        toolbar->setPageInfo(pagecount, pdfpage + 1);
    } else {
        toolbar->setPageInfo(pagecount);
    }
}

void MainWindow::rebuildLayerMenu() { layerVisibilityChanged(); }

void MainWindow::layerVisibilityChanged() {
    LayerController* lc = control->getLayerController();

    auto layer = lc->getCurrentLayerId();
    auto maxLayer = lc->getLayerCount();

    control->fireEnableAction(ACTION_DELETE_LAYER, layer > 0);
    control->fireEnableAction(ACTION_MERGE_LAYER_DOWN, layer > 1);
    control->fireEnableAction(ACTION_MOVE_SELECTION_LAYER_UP, layer < maxLayer);
    control->fireEnableAction(ACTION_MOVE_SELECTION_LAYER_DOWN, layer > 1);
    control->fireEnableAction(ACTION_GOTO_NEXT_LAYER, layer < maxLayer);
    control->fireEnableAction(ACTION_GOTO_PREVIOUS_LAYER, layer > 0);
    control->fireEnableAction(ACTION_GOTO_TOP_LAYER, layer < maxLayer);
}

auto MainWindow::getMenubar() const -> Menubar* { return menubar.get(); }

void MainWindow::show(GtkWindow* parent) { gtk_widget_show(this->window); }

void MainWindow::setUndoDescription(const string& description) { toolbar->setUndoDescription(description); }

void MainWindow::setRedoDescription(const string& description) { toolbar->setRedoDescription(description); }

auto MainWindow::getSpinPageNo() const -> SpinPageAdapter* { return toolbar->getPageSpinner(); }

auto MainWindow::getToolbarModel() const -> ToolbarModel* { return this->toolbar->getModel(); }

auto MainWindow::getToolMenuHandler() const -> ToolMenuHandler* { return this->toolbar.get(); }

void MainWindow::disableAudioPlaybackButtons() {
    setAudioPlaybackPaused(false);

    this->getToolMenuHandler()->disableAudioPlaybackButtons();
}

void MainWindow::enableAudioPlaybackButtons() { this->getToolMenuHandler()->enableAudioPlaybackButtons(); }

void MainWindow::setAudioPlaybackPaused(bool paused) { this->getToolMenuHandler()->setAudioPlaybackPaused(paused); }

void MainWindow::loadMainCSS(GladeSearchpath* gladeSearchPath, const gchar* cssFilename) {
    auto filepath = gladeSearchPath->findFile("", cssFilename);
    xoj::util::GObjectSPtr<GtkCssProvider> provider(gtk_css_provider_new(), xoj::util::adopt);
    gtk_css_provider_load_from_path(provider.get(), filepath.u8string().c_str(), nullptr);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(provider.get()),
                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

PdfFloatingToolbox* MainWindow::getPdfToolbox() const { return this->pdfFloatingToolBox.get(); }

FloatingToolbox* MainWindow::getFloatingToolbox() const { return this->floatingToolbox.get(); }
