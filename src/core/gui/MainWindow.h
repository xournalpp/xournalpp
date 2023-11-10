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
class PdfFloatingToolbox;
class OpacityToolbox;
class FloatingToolbox;
class GladeSearchpath;

class Menubar;

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

    /**
     * These methods are only used internally and for toolbar configuration
     */
    ToolbarData* clearToolbar();
    void loadToolbar(ToolbarData* d);


    void updatePageNumbers(size_t page, size_t pagecount, size_t pdfpage);

    void saveSidebarSize();

    void setMaximized(bool maximized);
    bool isMaximized() const;

    void setFullscreen(bool enabled) const;

    XournalView* getXournal() const;

    void setMenubarVisible(bool visible);
    void setSidebarVisible(bool visible);
    void setToolbarVisible(bool visible);

    Control* getControl() const;

    PdfFloatingToolbox* getPdfToolbox() const;
    OpacityToolbox* getOpacityToolbox() const;
    FloatingToolbox* getFloatingToolbox() const;

    void updateScrollbarSidebarPosition();

    void setUndoDescription(const std::string& description);
    void setRedoDescription(const std::string& description);

    SpinPageAdapter* getSpinPageNo() const;
    ToolbarModel* getToolbarModel() const;
    ToolMenuHandler* getToolMenuHandler() const;

    void setDynamicallyGeneratedSubmenuDisabled(bool disabled);

    void updateToolbarMenu();
    void updateColorscheme();

    const ToolbarWidgetArray& getToolbarWidgets() const;
    const char* getToolbarName(GtkToolbar* toolbar) const;

    Layout* getLayout() const;

    [[maybe_unused]] Menubar* getMenubar() const;

    /**
     * Get the position of the top left corner of screen (X11) or the window (Wayland)
     * relative to the Xournal Widget top left corner
     *
     * @see Util::toWidgetCoords()
     */
    utl::Point<double> getNegativeXournalWidgetPos() const;

    /**
     * Disable kinetic scrolling if there is a touchscreen device that was manually mapped to another enabled input
     * device class. This is required so the GtkScrolledWindow does not swallow all the events.
     */
    void setGtkTouchscreenScrollingForDeviceMapping();
    void setGtkTouchscreenScrollingEnabled(bool enabled);

private:
    void initXournalWidget();

    void createToolbar();

    static void buttonCloseSidebarClicked(GtkButton* button, MainWindow* win);

    /**
     * Window close Button is pressed
     */
    static bool deleteEventCallback(GtkWidget* widget, GdkEvent* event, Control* control);

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

    std::unique_ptr<XournalView> xournal;
    GtkWidget* winXournal = nullptr;
    std::unique_ptr<ScrollHandling> scrollHandling;

    std::atomic_bool gtkTouchscreenScrollingEnabled{true};

    std::unique_ptr<PdfFloatingToolbox> pdfFloatingToolBox;
    std::unique_ptr<OpacityToolbox> opacityToolBox;
    std::unique_ptr<FloatingToolbox> floatingToolbox;

    // Toolbars
    std::unique_ptr<ToolMenuHandler> toolbar;
    ToolbarData* selectedToolbar = nullptr;

    std::unique_ptr<Menubar> menubar;

    bool maximized = false;

    ToolbarWidgetArray toolbarWidgets;

    GtkAccelGroup* globalAccelGroup;

    bool sidebarVisible = true;

    xoj::util::WidgetSPtr boxContainerWidget;
    xoj::util::WidgetSPtr panedContainerWidget;
    xoj::util::WidgetSPtr mainContentWidget;
    xoj::util::WidgetSPtr sidebarWidget;
};
