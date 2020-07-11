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

#include "control/layer/LayerCtrlListener.h"
#include "gui/FloatingToolbox.h"
#include "model/Font.h"

#include "GladeGui.h"

class Control;
class Layout;
class MainWindow;
class SpinPageAdapter;
class ScrollHandling;
class ToolMenuHandler;
class ToolbarData;
class ToolbarModel;
class XournalView;
class MainWindowToolbarMenu;


class MainWindow: public GladeGui, public LayerCtrlListener {
public:
    MainWindow(GladeSearchpath* gladeSearchPath, Control* control);
    virtual ~MainWindow();

    // LayerCtrlListener
public:
    virtual void rebuildLayerMenu();
    virtual void layerVisibilityChanged();

    FloatingToolbox* floatingToolbox;

public:
    virtual void show(GtkWindow* parent);

    void setRecentMenu(GtkWidget* submenu);
    void toolbarSelected(ToolbarData* d);
    ToolbarData* getSelectedToolbar();
    void reloadToolbars();

    /**
     * This methods are only used internally and for toolbar configuration
     */
    ToolbarData* clearToolbar();
    void loadToolbar(ToolbarData* d);


    void updatePageNumbers(size_t page, size_t pagecount, size_t pdfpage);

    void setFontButtonFont(XojFont& font);
    XojFont getFontButtonFont();

    void saveSidebarSize();

    void setMaximized(bool maximized);
    bool isMaximized() const;

    XournalView* getXournal();

    void setSidebarVisible(bool visible);
    void setToolbarVisible(bool visible);

    Control* getControl();

    void updateScrollbarSidebarPosition();

    void setUndoDescription(const string& description);
    void setRedoDescription(const string& description);

    SpinPageAdapter* getSpinPageNo();
    ToolbarModel* getToolbarModel();
    ToolMenuHandler* getToolMenuHandler();

    void disableAudioPlaybackButtons();
    void enableAudioPlaybackButtons();
    void setAudioPlaybackPaused(bool paused);

    void setControlTmpDisabled(bool disabled);

    void updateToolbarMenu();

    GtkWidget** getToolbarWidgets(int& length);
    const char* getToolbarName(GtkToolbar* toolbar);

    Layout* getLayout();

    bool isGestureActive();


    /**
     * Disable kinetic scrolling if there is a touchscreen device that was manually mapped to another enabled input
     * device class. This is required so the GtkScrolledWindow does not swallow all the events.
     */
    void setTouchscreenScrollingForDeviceMapping();

private:
    void initXournalWidget();

    /**
     * Allow to hide menubar, but only if global menu is not enabled
     */
    void initHideMenu();
    static void toggleMenuBar(MainWindow* win);

    void createToolbarAndMenu();


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

    // Toolbars
    ToolMenuHandler* toolbar;
    ToolbarData* selectedToolbar = nullptr;
    bool toolbarIntialized = false;

    bool maximized = false;

    GtkWidget** toolbarWidgets;

    MainWindowToolbarMenu* toolbarSelectMenu;

    /**
     * Workaround for double hide menubar event
     */
    bool ignoreNextHideEvent;
};
