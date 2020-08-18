#include "MainWindow.h"

#include <config-dev.h>
#include <config-features.h>
#include <config.h>

#include "control/Control.h"
#include "control/layer/LayerController.h"
#include "gui/GladeSearchpath.h"
#include "gui/scroll/ScrollHandlingGtk.h"
#include "gui/scroll/ScrollHandlingXournalpp.h"
#include "toolbarMenubar/ToolMenuHandler.h"
#include "toolbarMenubar/model/ToolbarData.h"
#include "toolbarMenubar/model/ToolbarModel.h"
#include "widgets/SpinPageAdapter.h"
#include "widgets/XournalWidget.h"

#include "Layout.h"
#include "MainWindowToolbarMenu.h"
#include "ToolbarDefinitions.h"
#include "ToolitemDragDrop.h"
#include "XojMsgBox.h"
#include "XournalView.h"
#include "i18n.h"

#ifdef MAC_INTEGRATION
#include <gtkosxapplication.h>
#endif

#include <utility>

#include <gdk/gdk.h>

#include "gui/inputdevices/InputEvents.h"
#include "util/DeviceListHelper.h"

MainWindow::MainWindow(GladeSearchpath* gladeSearchPath, Control* control):
        GladeGui(gladeSearchPath, "main.glade", "mainWindow"), ignoreNextHideEvent(false) {
    this->control = control;
    this->toolbarWidgets = new GtkWidget*[TOOLBAR_DEFINITIONS_LEN];
    this->toolbarSelectMenu = new MainWindowToolbarMenu(this);

    loadMainCSS(gladeSearchPath, "xournalpp.css");

    GtkOverlay* overlay = GTK_OVERLAY(get("mainOverlay"));
    this->floatingToolbox = new FloatingToolbox(this, overlay);


    for (int i = 0; i < TOOLBAR_DEFINITIONS_LEN; i++) {
        GtkWidget* w = get(TOOLBAR_DEFINITIONS[i].guiName);
        g_object_ref(w);
        this->toolbarWidgets[i] = w;
    }

    initXournalWidget();

    setSidebarVisible(control->getSettings()->isSidebarVisible());

    // Window handler
    g_signal_connect(this->window, "delete-event", G_CALLBACK(deleteEventCallback), this->control);
    g_signal_connect(this->window, "window_state_event", G_CALLBACK(windowStateEventCallback), this);

    g_signal_connect(get("buttonCloseSidebar"), "clicked", G_CALLBACK(buttonCloseSidebarClicked), this);


    // "watch over" all events
    g_signal_connect(this->window, "key-press-event", G_CALLBACK(onKeyPressCallback), this);

    this->toolbar = new ToolMenuHandler(this->control, this, GTK_WINDOW(getWindow()));

    auto file = gladeSearchPath->findFile("", "toolbar.ini");

    ToolbarModel* tbModel = this->toolbar->getModel();

    if (!tbModel->parse(file, true)) {

        string msg = FS(_F("Could not parse general toolbar.ini file: {1}\n"
                           "No Toolbars will be available") %
                        file.string());
        XojMsgBox::showErrorToUser(control->getGtkWindow(), msg);
    }

    file = Util::getConfigFile(TOOLBAR_CONFIG);
    if (fs::exists(file)) {
        if (!tbModel->parse(file, false)) {
            string msg = FS(_F("Could not parse custom toolbar.ini file: {1}\n"
                               "Toolbars will not be available") %
                            file.string());
            XojMsgBox::showErrorToUser(control->getGtkWindow(), msg);
        }
    }

    createToolbarAndMenu();

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

#ifdef MAC_INTEGRATION
    GtkosxApplication* osxApp = gtkosx_application_get();

    GtkWidget* menubar = get("mainMenubar");
    gtk_widget_hide(menubar);
    gtkosx_application_set_menu_bar(osxApp, GTK_MENU_SHELL(menubar));

    g_signal_connect(osxApp, "NSApplicationWillTerminate",
                     G_CALLBACK(+[](GtkosxApplication* osxApp, MainWindow* self) { self->control->quit(false); }),
                     this);

    g_signal_connect(osxApp, "NSApplicationOpenFile",
                     G_CALLBACK(+[](GtkosxApplication* osxApp, char* path, MainWindow* self) {
                         return self->control->openFile(path);
                     }),
                     this);

    gtkosx_application_ready(osxApp);
#endif
}

MainWindow::~MainWindow() {
    for (int i = 0; i < TOOLBAR_DEFINITIONS_LEN; i++) {
        g_object_unref(this->toolbarWidgets[i]);
    }

    delete[] this->toolbarWidgets;
    this->toolbarWidgets = nullptr;

    delete this->toolbarSelectMenu;
    this->toolbarSelectMenu = nullptr;

    delete this->floatingToolbox;
    this->floatingToolbox = nullptr;

    delete this->xournal;
    this->xournal = nullptr;

    delete this->toolbar;
    this->toolbar = nullptr;

    delete scrollHandling;
    scrollHandling = nullptr;
}

/**
 * Topmost widgets, to check if there is a menu above
 */
const char* TOP_WIDGETS[] = {"tbTop1", "tbTop2", "mainContainerBox", nullptr};


void MainWindow::toggleMenuBar(MainWindow* win) {
    if (win->ignoreNextHideEvent) {
        win->ignoreNextHideEvent = false;
        return;
    }

    GtkWidget* menu = win->get("mainMenubar");
    if (gtk_widget_is_visible(menu)) {
        gtk_widget_hide(menu);
    } else {
        gtk_widget_show(menu);
        win->ignoreNextHideEvent = true;
    }
}

void MainWindow::initXournalWidget() {
    GtkWidget* boxContents = get("boxContents");

    if (control->getSettings()->isTouchWorkaround()) {
        GtkWidget* box1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        gtk_container_add(GTK_CONTAINER(boxContents), box1);

        GtkWidget* box2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_container_add(GTK_CONTAINER(box1), box2);

        scrollHandling = new ScrollHandlingXournalpp();

        this->xournal = new XournalView(box2, control, scrollHandling);

        gtk_container_add(GTK_CONTAINER(box2),
                          gtk_scrollbar_new(GTK_ORIENTATION_VERTICAL, scrollHandling->getVertical()));
        gtk_container_add(GTK_CONTAINER(box1),
                          gtk_scrollbar_new(GTK_ORIENTATION_HORIZONTAL, scrollHandling->getHorizontal()));

        control->getZoomControl()->initZoomHandler(box2, xournal, control);
        gtk_widget_show_all(box1);
    } else {
        winXournal = gtk_scrolled_window_new(nullptr, nullptr);

        setTouchscreenScrollingForDeviceMapping();

        gtk_container_add(GTK_CONTAINER(boxContents), winXournal);

        GtkWidget* vpXournal = gtk_viewport_new(nullptr, nullptr);

        gtk_container_add(GTK_CONTAINER(winXournal), vpXournal);

        scrollHandling = new ScrollHandlingGtk(GTK_SCROLLABLE(vpXournal));

        this->xournal = new XournalView(vpXournal, control, scrollHandling);

        control->getZoomControl()->initZoomHandler(winXournal, xournal, control);
        gtk_widget_show_all(winXournal);
    }
    // Todo configure-event

    Layout* layout = gtk_xournal_get_layout(this->xournal->getWidget());
    scrollHandling->init(this->xournal->getWidget(), layout);
}

void MainWindow::setTouchscreenScrollingForDeviceMapping() {
    for (InputDevice const& inputDevice: DeviceListHelper::getDeviceList(this->getControl()->getSettings())) {
        InputDeviceClass deviceClass = InputEvents::translateDeviceType(inputDevice.getName(), inputDevice.getSource(),
                                                                        this->getControl()->getSettings());
        if (inputDevice.getSource() == GDK_SOURCE_TOUCHSCREEN && deviceClass != INPUT_DEVICE_TOUCHSCREEN) {
            gtk_scrolled_window_set_kinetic_scrolling(GTK_SCROLLED_WINDOW(winXournal), false);
            break;
        }
    }
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

        GtkAccelGroup* accelGroup = gtk_accel_group_new();
        gtk_accel_group_connect(accelGroup, GDK_KEY_F10, static_cast<GdkModifierType>(0), GTK_ACCEL_VISIBLE,
                                g_cclosure_new_swap(G_CALLBACK(toggleMenuBar), this, nullptr));
        gtk_window_add_accel_group(GTK_WINDOW(getWindow()), accelGroup);
    }

    // Hide menubar at startup if specified in settings
    Settings* settings = control->getSettings();
    if (settings && !settings->isMenubarVisible()) {
        toggleMenuBar(this);
    }
}

auto MainWindow::getLayout() -> Layout* { return gtk_xournal_get_layout(GTK_WIDGET(this->xournal->getWidget())); }

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

    GdkPixbuf* image = gtk_selection_data_get_pixbuf(data);
    if (image) {
        win->control->clipboardPasteImage(image);

        g_object_unref(image);
        gtk_drag_finish(dragContext, true, false, time);
        return;
    }

    gchar** uris = gtk_selection_data_get_uris(data);
    if (uris) {
        for (int i = 0; uris[i] != nullptr && i < 3; i++) {
            const char* uri = uris[i];

            GCancellable* cancel = g_cancellable_new();
            int cancelTimeout = g_timeout_add(3000, reinterpret_cast<GSourceFunc>(cancellable_cancel), cancel);

            GFile* file = g_file_new_for_uri(uri);
            GError* err = nullptr;
            GFileInputStream* in = g_file_read(file, cancel, &err);
            if (g_cancellable_is_cancelled(cancel)) {
                continue;
            }

            g_object_unref(file);
            if (err == nullptr) {
                GdkPixbuf* pixbuf = gdk_pixbuf_new_from_stream(G_INPUT_STREAM(in), cancel, nullptr);
                if (g_cancellable_is_cancelled(cancel)) {
                    continue;
                }
                g_input_stream_close(G_INPUT_STREAM(in), cancel, nullptr);
                if (g_cancellable_is_cancelled(cancel)) {
                    continue;
                }

                if (pixbuf) {
                    win->control->clipboardPasteImage(pixbuf);

                    g_object_unref(pixbuf);
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

auto MainWindow::getControl() -> Control* { return control; }

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

auto MainWindow::onKeyPressCallback(GtkWidget* widget, GdkEventKey* event, MainWindow* win) -> bool {

    if (win->getXournal()->getSelection()) {
        // something is selected - give that control
        return false;
    }
    if (win->getXournal()->getTextEditor()) {
        // editing text - give that control
        return false;
    }
    if (event->keyval == GDK_KEY_Escape) {
        win->getControl()->getSearchBar()->showSearchBar(false);
        return true;
    }


    return false;
}

auto MainWindow::deleteEventCallback(GtkWidget* widget, GdkEvent* event, Control* control) -> bool {
    control->quit();

    return true;
}

void MainWindow::setSidebarVisible(bool visible) {
    Settings* settings = control->getSettings();
    GtkWidget* sidebar = get("sidebar");
    GtkWidget* panel = get("panelMainContents");

    gtk_widget_set_visible(sidebar, visible);
    settings->setSidebarVisible(visible);

    if (!visible && (control->getSidebar() != nullptr)) {
        saveSidebarSize();
    }

    if (visible) {
        gtk_paned_set_position(GTK_PANED(panel), settings->getSidebarWidth());
    }

    GtkWidget* w = get("menuViewSidebarVisible");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(w), visible);
}

void MainWindow::setToolbarVisible(bool visible) {
    Settings* settings = control->getSettings();

    settings->setToolbarVisible(visible);
    for (int i = 0; i < TOOLBAR_DEFINITIONS_LEN; i++) {
        gtk_widget_set_visible(this->toolbarWidgets[i], visible);
    }

    GtkWidget* w = get("menuViewToolbarsVisible");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(w), visible);
}

void MainWindow::saveSidebarSize() {
    GtkWidget* panel = get("panelMainContents");

    this->control->getSettings()->setSidebarWidth(gtk_paned_get_position(GTK_PANED(panel)));
}

void MainWindow::setMaximized(bool maximized) { this->maximized = maximized; }

auto MainWindow::isMaximized() const -> bool { return this->maximized; }

auto MainWindow::getXournal() -> XournalView* { return xournal; }

auto MainWindow::windowStateEventCallback(GtkWidget* window, GdkEventWindowState* event, MainWindow* win) -> bool {
    win->setMaximized(gtk_window_is_maximized(GTK_WINDOW(window)));

    return false;
}

void MainWindow::reloadToolbars() {
    bool inDragAndDrop = this->control->isInDragAndDropToolbar();

    ToolbarData* d = getSelectedToolbar();

    if (inDragAndDrop) {
        this->control->endDragDropToolbar();
    }

    this->clearToolbar();
    this->toolbarSelected(d);

    if (inDragAndDrop) {
        this->control->startDragDropToolbar();
    }
}

void MainWindow::toolbarSelected(ToolbarData* d) {
    if (!this->toolbarIntialized || this->selectedToolbar == d) {
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
            ToolMenuHandler::unloadToolbar(this->toolbarWidgets[i]);
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
        this->toolbar->load(d, this->toolbarWidgets[i], TOOLBAR_DEFINITIONS[i].propName,
                            TOOLBAR_DEFINITIONS[i].horizontal);
    }

    this->floatingToolbox->flagRecalculateSizeRequired();
}

auto MainWindow::getSelectedToolbar() -> ToolbarData* { return this->selectedToolbar; }

auto MainWindow::getToolbarWidgets(int& length) -> GtkWidget** {
    length = TOOLBAR_DEFINITIONS_LEN;
    return this->toolbarWidgets;
}

auto MainWindow::getToolbarName(GtkToolbar* toolbar) -> const char* {
    for (int i = 0; i < TOOLBAR_DEFINITIONS_LEN; i++) {
        if (static_cast<void*>(this->toolbarWidgets[i]) == static_cast<void*>(toolbar)) {
            return TOOLBAR_DEFINITIONS[i].propName;
        }
    }

    return "";
}

void MainWindow::setControlTmpDisabled(bool disabled) {
    toolbar->setTmpDisabled(disabled);
    toolbarSelectMenu->setTmpDisabled(disabled);

    GtkWidget* menuFileRecent = get("menuFileRecent");
    gtk_widget_set_sensitive(menuFileRecent, !disabled);
}

void MainWindow::updateToolbarMenu() { createToolbarAndMenu(); }

void MainWindow::createToolbarAndMenu() {
    GtkMenuShell* menubar = GTK_MENU_SHELL(get("menuViewToolbar"));
    g_return_if_fail(menubar != nullptr);

    toolbarSelectMenu->updateToolbarMenu(menubar, control->getSettings(), toolbar);

    ToolbarData* td = toolbarSelectMenu->getSelectedToolbar();
    if (td) {
        this->toolbarIntialized = true;
        toolbarSelected(td);
    }

    if (!this->control->getAudioController()->isPlaying()) {
        this->getToolMenuHandler()->disableAudioPlaybackButtons();
    }

    this->control->getScheduler()->unblockRerenderZoom();
}

void MainWindow::setFontButtonFont(XojFont& font) { toolbar->setFontButtonFont(font); }

auto MainWindow::getFontButtonFont() -> XojFont { return toolbar->getFontButtonFont(); }

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

    string pdfText;
    if (pdfpage != npos) {
        pdfText = string(", ") + FS(_F("PDF Page {1}") % (pdfpage + 1));
    }
    toolbar->setPageText(FS(C_F("Page {pagenumber} \"of {pagecount}\"", " of {1}{2}") % pagecount % pdfText));
}

void MainWindow::rebuildLayerMenu() { layerVisibilityChanged(); }

void MainWindow::layerVisibilityChanged() {
    LayerController* lc = control->getLayerController();

    int layer = lc->getCurrentLayerId();
    int maxLayer = lc->getLayerCount();

    control->fireEnableAction(ACTION_DELETE_LAYER, layer > 0);
    control->fireEnableAction(ACTION_GOTO_NEXT_LAYER, layer < maxLayer);
    control->fireEnableAction(ACTION_GOTO_PREVIOUS_LAYER, layer > 0);
    control->fireEnableAction(ACTION_GOTO_TOP_LAYER, layer < maxLayer);
}

void MainWindow::setRecentMenu(GtkWidget* submenu) {
    GtkWidget* menuitem = get("menuFileRecent");
    g_return_if_fail(menuitem != nullptr);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem), submenu);
}

void MainWindow::show(GtkWindow* parent) { gtk_widget_show(this->window); }

void MainWindow::setUndoDescription(const string& description) { toolbar->setUndoDescription(description); }

void MainWindow::setRedoDescription(const string& description) { toolbar->setRedoDescription(description); }

auto MainWindow::getSpinPageNo() -> SpinPageAdapter* { return toolbar->getPageSpinner(); }

auto MainWindow::getToolbarModel() -> ToolbarModel* { return this->toolbar->getModel(); }

auto MainWindow::getToolMenuHandler() -> ToolMenuHandler* { return this->toolbar; }

void MainWindow::disableAudioPlaybackButtons() {
    setAudioPlaybackPaused(false);

    this->getToolMenuHandler()->disableAudioPlaybackButtons();
}

void MainWindow::enableAudioPlaybackButtons() { this->getToolMenuHandler()->enableAudioPlaybackButtons(); }

void MainWindow::setAudioPlaybackPaused(bool paused) { this->getToolMenuHandler()->setAudioPlaybackPaused(paused); }

void MainWindow::loadMainCSS(GladeSearchpath* gladeSearchPath, const gchar* cssFilename) {
    auto filepath = gladeSearchPath->findFile("", cssFilename);
    GtkCssProvider* provider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(provider, filepath.u8string().c_str(), nullptr);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
}
