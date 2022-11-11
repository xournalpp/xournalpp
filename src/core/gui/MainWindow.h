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

#include <atomic>   // for atomic_bool
#include <cstddef>  // for size_t
#include <memory>   // for unique_ptr
#include <string>   // for string

#include <gdk/gdk.h>      // for GdkDragContext, GdkEvent
#include <glib-object.h>  // for GClosure
#include <glib.h>         // for gpointer, gboolean, gint
#include <gtk/gtk.h>      // for GtkWidget, GtkCheckMenu...

#include "control/layer/LayerCtrlListener.h"  // for LayerCtrlListener
#include "model/Font.h"                       // for XojFont
#include "util/raii/GObjectSPtr.h"

#include "GladeGui.h"  // for GladeGui

class Control;
class Layout;
class SpinPageAdapter;
class ScrollHandling;
class ToolMenuHandler;
class ToolbarData;
class ToolbarModel;
class XournalView;
class MainWindowToolbarMenu;
class PdfFloatingToolbox;
class FloatingToolbox;
class GladeSearchpath;

class MainWindow: public GladeGui, public LayerCtrlListener {
public:
    MainWindow(GladeSearchpath* gladeSearchPath, Control* control);
    ~MainWindow() override;

    // LayerCtrlListener
public:
    void rebuildLayerMenu() override;
    void layerVisibilityChanged() override;

    FloatingToolbox* floatingToolbox;
public:
    void show(GtkWindow* parent) override;

    void setRecentMenu(GtkWidget* submenu);
    void toolbarSelected(ToolbarData* d);
    ToolbarData* getSelectedToolbar() const;
    [[maybe_unused]] void reloadToolbars();

    /**
     * These methods are only used internally and for toolbar configuration
     */
    ToolbarData* clearToolbar();
    void loadToolbar(ToolbarData* d);


    void updatePageNumbers(size_t page, size_t pagecount, size_t pdfpage);

    void setFontButtonFont(XojFont& font);
    XojFont getFontButtonFont() const;

    void saveSidebarSize();

    void setMaximized(bool maximized);
    bool isMaximized() const;

    XournalView* getXournal() const;

    void setSidebarVisible(bool visible);
    void setToolbarVisible(bool visible);

    Control* getControl() const;

    PdfFloatingToolbox* getPdfToolbox() const;

    void updateScrollbarSidebarPosition();

    void setUndoDescription(const std::string& description);
    void setRedoDescription(const std::string& description);

    SpinPageAdapter* getSpinPageNo() const;
    ToolbarModel* getToolbarModel() const;
    ToolMenuHandler* getToolMenuHandler() const;

    void disableAudioPlaybackButtons();
    void enableAudioPlaybackButtons();
    void setAudioPlaybackPaused(bool paused);

    void setControlTmpDisabled(bool disabled);

    void updateToolbarMenu();
    void updateColorscheme();

    GtkWidget** getToolbarWidgets(int& length) const;
    const char* getToolbarName(GtkToolbar* toolbar) const;

    Layout* getLayout() const;

    bool isGestureActive() const;


    /**
     * Disable kinetic scrolling if there is a touchscreen device that was manually mapped to another enabled input
     * device class. This is required so the GtkScrolledWindow does not swallow all the events.
     */
    void setGtkTouchscreenScrollingForDeviceMapping();
    void setGtkTouchscreenScrollingEnabled(bool enabled);

    void rebindMenubarAccelerators();

private:
    void initXournalWidget();

    /**
     * Allow to hide menubar, but only if global menu is not enabled
     */
    void initHideMenu();
    static void toggleMenuBar(MainWindow* win);

    void createToolbarAndMenu();
    static void rebindAcceleratorsMenuItem(GtkWidget* widget, gpointer user_data);
    static void rebindAcceleratorsSubMenu(GtkWidget* widget, gpointer user_data);
    static gboolean isKeyForClosure(GtkAccelKey* key, GClosure* closure, gpointer data);
    static gboolean invokeMenu(GtkWidget* widget);

    static void buttonCloseSidebarClicked(GtkButton* button, MainWindow* win);

    /**
     * Sidebar show / hidden
     */
    static void viewShowSidebar(GtkCheckMenuItem* checkmenuitem, MainWindow* win);

    /**
     * Toolbar show / hidden
     */
    static void viewShowToolbar(GtkCheckMenuItem* checkmenuitem, MainWindow* win);

    /**
     * Window close Button is pressed
     */
    static bool deleteEventCallback(GtkWidget* widget, GdkEvent* event, Control* control);

    /**
     * Key is pressed
     */
    static bool onKeyPressCallback(GtkWidget* widget, GdkEventKey* event, MainWindow* win);

    /**
     * Callback fro window states, we ned to know if the window is fullscreen
     */
    static bool windowStateEventCallback(GtkWidget* window, GdkEventWindowState* event, MainWindow* win);

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

    XournalView* xournal = nullptr;
    GtkWidget* winXournal = nullptr;
    ScrollHandling* scrollHandling = nullptr;

    std::atomic_bool gtkTouchscreenScrollingEnabled{true};

    std::unique_ptr<PdfFloatingToolbox> pdfFloatingToolBox;

    // Toolbars
    std::unique_ptr<ToolMenuHandler> toolbar;
    ToolbarData* selectedToolbar = nullptr;
    bool toolbarIntialized = false;

    bool maximized = false;

    GtkWidget** toolbarWidgets;

    MainWindowToolbarMenu* toolbarSelectMenu;
    GtkAccelGroup* globalAccelGroup;

    bool sidebarVisible = true;

    xoj::util::WidgetSPtr boxContainerWidget;
    xoj::util::WidgetSPtr panedContainerWidget;
    xoj::util::WidgetSPtr mainContentWidget;
    xoj::util::WidgetSPtr sidebarWidget;
};
