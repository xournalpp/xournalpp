/*
 * Xournal++
 *
 * The Main window
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <array>    // for array
#include <atomic>   // for atomic_bool
#include <cstddef>  // for size_t
#include <memory>   // for unique_ptr
#include <string>   // for string

#include <gdk/gdk.h>      // for GdkDragContext, GdkEvent
#include <glib-object.h>  // for GClosure
#include <glib.h>         // for gpointer, gboolean, gint
#include <gtk/gtk.h>      // for GtkWidget, GtkCheckMenu...

#include "util/Point.h"
#include "util/raii/GObjectSPtr.h"

#include "GladeGui.h"            // for GladeGui
#include "ToolbarDefinitions.h"  // for TOOLBAR_DEFINITIONS_LEN

class Control;
class Layout;
class SpinPageAdapter;
class ScrollHandling;
class ToolMenuHandler;
class ToolbarData;
class ToolbarModel;
class XournalView;
class ZoomControl;
class PdfFloatingToolbox;
class FloatingToolbox;
class GladeSearchpath;

class Menubar;
class Sidebar;

typedef std::array<xoj::util::WidgetSPtr, TOOLBAR_DEFINITIONS_LEN> ToolbarWidgetArray;

class MainWindow: public GladeGui {
public:
    MainWindow(GladeSearchpath* gladeSearchPath, Control* control, GtkApplication* parent);
    ~MainWindow() override;

    void populate(GladeSearchpath* gladeSearchPath);

public:
    GMenuModel* getMenuModel() const;

    void show(GtkWindow* parent) override;

    void toolbarSelected(const std::string& id);
    void toolbarSelected(ToolbarData* d);
    ToolbarData* getSelectedToolbar() const;

private:
    const ToolbarData* clearToolbar();
    void loadToolbar(ToolbarData* d);

public:
    /**
     * reloadToolbars reloads the currently selected toolbar
     *
     * This is especially useful when a change in the SettingsDialog should be reflected right away
     */
    void reloadToolbars();

    void updatePageNumbers(size_t page, size_t pagecount, size_t pdfpage);

    void setMaximized(bool maximized);
    bool isMaximized() const;

    void setFullscreen(bool enabled) const;

    bool isDarkTheme() const;

    XournalView* getXournal() const;
    ZoomControl* getZoomControl() const;

    void createSidebar();
    Sidebar* getSidebar() const;

    /// "win.new-window" must be added after the window action map exists.
    void registerNewWindowAction();

    void setMenubarVisible(bool visible);
    void setSidebarVisible(bool visible);
    void setToolbarVisible(bool visible);

    Control* getControl() const;

    PdfFloatingToolbox* getPdfToolbox() const;
    FloatingToolbox* getFloatingToolbox() const;

    void updateScrollbarSidebarPosition();

    void setUndoDescription(const std::string& description);
    void setRedoDescription(const std::string& description);

    ToolbarModel* getToolbarModel() const;
    ToolMenuHandler* getToolMenuHandler() const;

    void setDynamicallyGeneratedSubmenuDisabled(bool disabled);

    void updateToolbarMenu();
    /// Updates the shared GTK theme, then each window's style class.
    void updateColorscheme();
    /// Applies the current theme to this window only. Does not touch GtkSettings.
    void applyWindowColorscheme();

    const ToolbarWidgetArray& getToolbarWidgets() const;
    const char* getToolbarName(GtkToolbar* toolbar) const;

    Layout* getLayout() const;

    [[maybe_unused]] Menubar* getMenubar() const;
    /// Hands the menubar to Control so it can stay the application menu after this window closes.
    std::unique_ptr<Menubar> releaseMenubar();

    /**
     * Disable kinetic scrolling if there is a touchscreen device that was manually mapped to another enabled input
     * device class. This is required so the GtkScrolledWindow does not swallow all the events.
     */
    void setGtkTouchscreenScrollingForDeviceMapping();
    void setGtkTouchscreenScrollingEnabled(bool enabled);

    /// Infer the window's DPI from available monitor info and use it to set the default zoom value.
    void setDPI() const;

private:
    void initXournalWidget();

    void createToolbar();

    /**
     * Update the position of the separator in the paned container, adjusting it to the saved sidebar width.
     * @param contentWidth should be the width of the paned container. The caller should retrieve the width
     * of the container before any modifications to it, as that will reset its allocation.
     */
    void updatePanedPosition(int contentWidth);

    /**
     * Window close Button is pressed
     */
    static bool deleteEventCallback(GtkWidget* widget, GdkEvent* event, MainWindow* win);

    /**
     * Window is maximized/minimized
     */
    static void windowMaximizedCallback(GObject* window, GParamSpec*, MainWindow* win);

    /**
     * Callback for drag & drop files
     */
    static void dragDataRecived(GtkWidget* widget, GdkDragContext* dragContext, gint x, gint y, GtkSelectionData* data,
                                guint info, guint time, MainWindow* win);

    /**
     * Load Overall CSS file with custom icons, other styling and potentially, user changes
     */
    static void loadMainCSS(GladeSearchpath* gladeSearchPath, const gchar* cssFilename);

private:
    Control* control;

    /// Owned here so each window keeps an independent zoom. Declared before views that use it.
    std::unique_ptr<ZoomControl> zoomControl;

    std::unique_ptr<XournalView> xournal;
    std::unique_ptr<Sidebar> sidebar;
    GtkWidget* winXournal = nullptr;
    std::unique_ptr<ScrollHandling> scrollHandling;

    std::atomic_bool gtkTouchscreenScrollingEnabled{true};

    std::unique_ptr<PdfFloatingToolbox> pdfFloatingToolBox;
    std::unique_ptr<FloatingToolbox> floatingToolbox;

    // Toolbars
    std::unique_ptr<ToolMenuHandler> toolbar;
    ToolbarData* selectedToolbar = nullptr;

    std::unique_ptr<Menubar> menubar;

    bool maximized = false;
    bool darkMode = false;
    bool modifiedGtkSettingsTheme = false;
    /// GtkSettings is process-wide. These handlers must be disconnected before this window is freed.
    gulong themeNameHandler = 0;
    gulong themeDarkHandler = 0;

    /// The menubar GTK shows for every window, which may outlive this one.
    Menubar* menuForDisplay() const;

    ToolbarWidgetArray toolbarWidgets;

    bool sidebarVisible = true;
    bool closing = false;

    /// The last monitor the window has been moved to -- used for setting dpi
    GdkMonitor* lastMonitor = nullptr;

    xoj::util::WidgetSPtr boxContainerWidget;
    xoj::util::WidgetSPtr panedContainerWidget;
    xoj::util::WidgetSPtr mainContentWidget;
    xoj::util::WidgetSPtr sidebarWidget;
};
