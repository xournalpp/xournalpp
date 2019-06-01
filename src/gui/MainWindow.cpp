#include "MainWindow.h"

#include "Layout.h"
#include "MainWindowToolbarMenu.h"
#include "ToolitemDragDrop.h"
#include "XournalView.h"

#include "control/Control.h"
#include "control/layer/LayerController.h"
#include "control/zoom/ZoomGesture.h"
#include "gui/GladeSearchpath.h"
#include "gui/scroll/ScrollHandlingGtk.h"
#include "gui/scroll/ScrollHandlingXournalpp.h"
#include "ToolbarDefinitions.h"
#include "toolbarMenubar/model/ToolbarData.h"
#include "toolbarMenubar/model/ToolbarModel.h"
#include "toolbarMenubar/ToolMenuHandler.h"
#include "widgets/SpinPageAdapter.h"
#include "widgets/XournalWidget.h"

#include <config.h>
#include <config-dev.h>
#include <config-features.h>
#include <i18n.h>
#include <XojMsgBox.h>

#ifdef MAC_INTEGRATION
#include <gtkosxapplication.h>
#endif

#include <gdk/gdk.h>
#include <util/DeviceListHelper.h>
#include <gui/inputdevices/InputEvents.h>

MainWindow::MainWindow(GladeSearchpath* gladeSearchPath, Control* control)
 : GladeGui(gladeSearchPath, "main.glade", "mainWindow"),
   ignoreNextHideEvent(false)
{
	XOJ_INIT_TYPE(MainWindow);

	this->control = control;
	this->toolbarWidgets = new GtkWidget*[TOOLBAR_DEFINITIONS_LEN];
	this->toolbarSelectMenu = new MainWindowToolbarMenu(this);

	
	
	GtkOverlay *overlay = GTK_OVERLAY (get("mainOverlay"));
	this->floatingToolbox = new FloatingToolbox (this, overlay);  

		
	for (int i = 0; i < TOOLBAR_DEFINITIONS_LEN; i++)
	{
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

	string file = gladeSearchPath->findFile("", "toolbar.ini");

	ToolbarModel* tbModel = this->toolbar->getModel();

	if (!tbModel->parse(file, true))
	{

		string msg = FS(_F("Could not parse general toolbar.ini file: {1}\n"
						   "No Toolbars will be available") % file);
		XojMsgBox::showErrorToUser(control->getGtkWindow(), msg);
	}

	file = string(g_get_home_dir()) + G_DIR_SEPARATOR_S + CONFIG_DIR + G_DIR_SEPARATOR_S + TOOLBAR_CONFIG;
	if (g_file_test(file.c_str(), G_FILE_TEST_EXISTS))
	{
		if (!tbModel->parse(file, false))
		{
			string msg = FS(_F("Could not parse custom toolbar.ini file: {1}\n"
							   "Toolbars will not be available") % file);
			XojMsgBox::showErrorToUser(control->getGtkWindow(), msg);
		}
	}

	createToolbarAndMenu();

	GtkWidget* menuViewSidebarVisible = get("menuViewSidebarVisible");
	g_signal_connect(menuViewSidebarVisible, "toggled", G_CALLBACK(viewShowSidebar), this);

	updateScrollbarSidebarPosition();

	gtk_window_set_default_size(GTK_WINDOW(this->window),
								control->getSettings()->getMainWndWidth(), control->getSettings()->getMainWndHeight());

	if (control->getSettings()->isMainWndMaximized())
	{
		gtk_window_maximize(GTK_WINDOW(this->window));
	}
	else
	{
		gtk_window_unmaximize(GTK_WINDOW(this->window));
	}

	getSpinPageNo()->addListener(this->control->getScrollHandler());


	Util::execInUiThread([=]() {
		// Execute after the window is visible, else the check won't work
		initHideMenu();
	});

	// Drag and Drop
	g_signal_connect(this->window, "drag-data-received", G_CALLBACK(dragDataRecived), this);

	gtk_drag_dest_set(this->window, GTK_DEST_DEFAULT_ALL, NULL, 0, GDK_ACTION_COPY);
	gtk_drag_dest_add_uri_targets(this->window);
	gtk_drag_dest_add_image_targets(this->window);
	gtk_drag_dest_add_text_targets(this->window);

	LayerCtrlListener::registerListener(control->getLayerController());

#ifdef MAC_INTEGRATION
	GtkosxApplication* osxApp = gtkosx_application_get();

	GtkWidget* menubar = get("mainMenubar");
	gtk_widget_hide(menubar);
	gtkosx_application_set_menu_bar(osxApp, GTK_MENU_SHELL(menubar));

	g_signal_connect(osxApp, "NSApplicationWillTerminate", G_CALLBACK(
		+[](GtkosxApplication* osxApp, MainWindow* self)
		{
			XOJ_CHECK_TYPE_OBJ(self, MainWindow);
			self->control->quit(false);
		}), this);

	g_signal_connect(osxApp, "NSApplicationOpenFile", G_CALLBACK(
		+[](GtkosxApplication* osxApp, char* path, MainWindow* self)
		{
			XOJ_CHECK_TYPE_OBJ(self, MainWindow);
			return self->control->openFile(path);
		}), this);

	gtkosx_application_ready(osxApp);
#endif
}

MainWindow::~MainWindow()
{
	XOJ_CHECK_TYPE(MainWindow);

	for (int i = 0; i < TOOLBAR_DEFINITIONS_LEN; i++)
	{
		g_object_unref(this->toolbarWidgets[i]);
	}

	delete[] this->toolbarWidgets;
	this->toolbarWidgets = NULL;

	delete this->toolbarSelectMenu;
	this->toolbarSelectMenu = NULL;

	delete this->floatingToolbox;
	this->floatingToolbox = NULL;
	
	delete this->xournal;
	this->xournal = NULL;

	delete this->toolbar;
	this->toolbar = NULL;

	delete this->zoomGesture;
	this->zoomGesture = NULL;

	delete scrollHandling;
	scrollHandling = NULL;

	XOJ_RELEASE_TYPE(MainWindow);
}

/**
 * Topmost widgets, to check if there is a menu above
 */
const char* TOP_WIDGETS[] = {"tbTop1", "tbTop2", "mainContainerBox", NULL};


void MainWindow::toggleMenuBar(MainWindow* win)
{
	XOJ_CHECK_TYPE_OBJ(win, MainWindow);

	if (win->ignoreNextHideEvent)
	{
		win->ignoreNextHideEvent = false;
		return;
	}

	GtkWidget* menu = win->get("mainMenubar");
	if (gtk_widget_is_visible(menu))
	{
		gtk_widget_hide(menu);
	}
	else
	{
		gtk_widget_show(menu);
		win->ignoreNextHideEvent = true;
	}
}

void MainWindow::initXournalWidget()
{
	XOJ_CHECK_TYPE(MainWindow);

	GtkWidget* boxContents = get("boxContents");

	if (control->getSettings()->isTouchWorkaround())
	{
		GtkWidget* box1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
		gtk_container_add(GTK_CONTAINER(boxContents), box1);

		GtkWidget* box2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
		gtk_container_add(GTK_CONTAINER(box1), box2);

		scrollHandling = new ScrollHandlingXournalpp();

		this->zoomGesture = new ZoomGesture(control->getZoomControl());

		this->xournal = new XournalView(box2, control, scrollHandling, zoomGesture);

		if (control->getSettings()->isZoomGesturesEnabled())
		{
			this->zoomGesture->connect(this->xournal->getWidget());
		}

		gtk_container_add(GTK_CONTAINER(box2), gtk_scrollbar_new(GTK_ORIENTATION_VERTICAL, scrollHandling->getVertical()));
		gtk_container_add(GTK_CONTAINER(box1), gtk_scrollbar_new(GTK_ORIENTATION_HORIZONTAL, scrollHandling->getHorizontal()));

		gtk_widget_show_all(box1);
	}
	else
	{
		winXournal = gtk_scrolled_window_new(NULL, NULL);

		setTouchscreenScrollingForDeviceMapping();

		gtk_container_add(GTK_CONTAINER(boxContents), winXournal);

		GtkWidget* vpXournal = gtk_viewport_new(NULL, NULL);

		gtk_container_add(GTK_CONTAINER(winXournal), vpXournal);

		scrollHandling = new ScrollHandlingGtk(GTK_SCROLLABLE(vpXournal));

		this->zoomGesture = new ZoomGesture(control->getZoomControl());

		this->xournal = new XournalView(vpXournal, control, scrollHandling, zoomGesture);

		if (control->getSettings()->isZoomGesturesEnabled())
		{
			this->zoomGesture->connect(winXournal);
		}

		gtk_widget_show_all(winXournal);
	}

	Layout* layout = gtk_xournal_get_layout(this->xournal->getWidget());
	scrollHandling->init(this->xournal->getWidget(), layout);
}

void MainWindow::setTouchscreenScrollingForDeviceMapping() {
	XOJ_CHECK_TYPE(MainWindow);

	auto deviceListHelper = new DeviceListHelper(false);
	vector<InputDevice> deviceList = deviceListHelper->getDeviceList();
	for(InputDevice inputDevice : deviceList)
	{
		GdkDevice* device = inputDevice.getDevice();
		int deviceClass = this->getControl()->getSettings()->getDeviceClassForDevice(device);
		if (gdk_device_get_source(device) == GDK_SOURCE_TOUCHSCREEN && deviceClass != INPUT_DEVICE_TOUCHSCREEN && deviceClass != INPUT_DEVICE_IGNORE)
		{
			gtk_scrolled_window_set_kinetic_scrolling(GTK_SCROLLED_WINDOW(winXournal), false);
		}
	}
}

/**
 * Allow to hide menubar, but only if global menu is not enabled
 */
void MainWindow::initHideMenu()
{
	XOJ_CHECK_TYPE(MainWindow);

	int top = -1;
	for (int i = 0; TOP_WIDGETS[i]; i++)
	{
		GtkWidget* w = get(TOP_WIDGETS[i]);
		GtkAllocation allocation;
		gtk_widget_get_allocation(w, &allocation);
		if (allocation.y != -1)
		{
			top = allocation.y;
			break;
		}
	}

	GtkWidget* menuItem = get("menuHideMenu");
	if (top < 5)
	{
		// There is no menu to hide, the menu is in the globalmenu!
		gtk_widget_hide(menuItem);
	}
	else
	{
		// Menu found, allow to hide it
		g_signal_connect(menuItem, "activate", G_CALLBACK(
			+[](GtkMenuItem* menuitem, MainWindow* self)
			{
				XOJ_CHECK_TYPE_OBJ(self, MainWindow);
				toggleMenuBar(self);
			}), this);

		GtkAccelGroup* accelGroup = gtk_accel_group_new();
		gtk_accel_group_connect(accelGroup, GDK_KEY_F10, (GdkModifierType) 0, GTK_ACCEL_VISIBLE,
				g_cclosure_new_swap(G_CALLBACK(toggleMenuBar), this, NULL));
		gtk_window_add_accel_group(GTK_WINDOW(getWindow()), accelGroup);
	}

	// Hide menubar at startup if specified in settings
	Settings* settings = control->getSettings();
	if (settings && !settings->isMenubarVisible())
	{
		toggleMenuBar(this);
	}
}

Layout* MainWindow::getLayout()
{
	XOJ_CHECK_TYPE(MainWindow);

	return gtk_xournal_get_layout(GTK_WIDGET(this->xournal->getWidget()));
}

bool MainWindow::isGestureActive()
{
	XOJ_CHECK_TYPE(MainWindow);

	if (zoomGesture == NULL) // Gestures disabled
	{
		return false;
	}

	return zoomGesture->isGestureActive();
}

bool cancellable_cancel(GCancellable* cancel)
{
	g_cancellable_cancel(cancel);

	g_warning("Timeout... Cancel loading URL");

	return false;
}

void MainWindow::dragDataRecived(GtkWidget* widget, GdkDragContext* dragContext, gint x, gint y,
								 GtkSelectionData* data, guint info, guint time, MainWindow* win)
{
	XOJ_CHECK_TYPE_OBJ(win, MainWindow);

	GtkWidget* source = gtk_drag_get_source_widget(dragContext);
	if (source && widget == gtk_widget_get_toplevel(source))
	{
		gtk_drag_finish(dragContext, false, false, time);
		return;
	}

	guchar* text = gtk_selection_data_get_text(data);
	if (text)
	{
		win->control->clipboardPasteText((const char*) text);

		g_free(text);
		gtk_drag_finish(dragContext, true, false, time);
		return;
	}

	GdkPixbuf* image = gtk_selection_data_get_pixbuf(data);
	if (image)
	{
		win->control->clipboardPasteImage(image);

		g_object_unref(image);
		gtk_drag_finish(dragContext, true, false, time);
		return;
	}

	gchar** uris = gtk_selection_data_get_uris(data);
	if (uris)
	{
		for (int i = 0; uris[i] != NULL && i < 3; i++)
		{
			const char* uri = uris[i];

			GCancellable* cancel = g_cancellable_new();
			int cancelTimeout = g_timeout_add(3000, (GSourceFunc) cancellable_cancel, cancel);

			GFile* file = g_file_new_for_uri(uri);
			GError* err = NULL;
			GFileInputStream* in = g_file_read(file, cancel, &err);
			if (g_cancellable_is_cancelled(cancel))
			{
				continue;
			}

			g_object_unref(file);
			if (err == NULL)
			{
				GdkPixbuf* pixbuf = gdk_pixbuf_new_from_stream(G_INPUT_STREAM(in), cancel, NULL);
				if (g_cancellable_is_cancelled(cancel))
				{
					continue;
				}
				g_input_stream_close(G_INPUT_STREAM(in), cancel, NULL);
				if (g_cancellable_is_cancelled(cancel))
				{
					continue;
				}

				if (pixbuf)
				{
					win->control->clipboardPasteImage(pixbuf);

					g_object_unref(pixbuf);
				}
			}
			else
			{
				g_error_free(err);
			}

			if (!g_cancellable_is_cancelled(cancel))
			{
				g_source_remove(cancelTimeout);
			}
			g_object_unref(cancel);
		}

		gtk_drag_finish(dragContext, true, false, time);

		g_strfreev(uris);
	}

	gtk_drag_finish(dragContext, false, false, time);
}

void MainWindow::viewShowSidebar(GtkCheckMenuItem* checkmenuitem, MainWindow* win)
{
	XOJ_CHECK_TYPE_OBJ(win, MainWindow);

	bool a = gtk_check_menu_item_get_active(checkmenuitem);
	if (win->control->getSettings()->isSidebarVisible() == a)
	{
		return;
	}
	win->setSidebarVisible(a);
}

Control* MainWindow::getControl()
{
	XOJ_CHECK_TYPE(MainWindow);

	return control;
}

void MainWindow::updateScrollbarSidebarPosition()
{
	XOJ_CHECK_TYPE(MainWindow);

	GtkWidget* panelMainContents = get("panelMainContents");

	if (winXournal != NULL)
	{
		GtkScrolledWindow* scrolledWindow = GTK_SCROLLED_WINDOW(winXournal);

		ScrollbarHideType type = this->getControl()->getSettings()->getScrollbarHideType();

		bool scrollbarOnLeft = control->getSettings()->isScrollbarOnLeft();
		if (scrollbarOnLeft)
		{
			gtk_scrolled_window_set_placement(scrolledWindow, GTK_CORNER_TOP_RIGHT);
		}
		else
		{
			gtk_scrolled_window_set_placement(scrolledWindow, GTK_CORNER_TOP_LEFT);
		}

		gtk_widget_set_visible(gtk_scrolled_window_get_hscrollbar(scrolledWindow), !(type & SCROLLBAR_HIDE_HORIZONTAL));
		gtk_widget_set_visible(gtk_scrolled_window_get_vscrollbar(scrolledWindow), !(type & SCROLLBAR_HIDE_VERTICAL));
	}

	GtkWidget* sidebar = get("sidebar");
	GtkWidget* boxContents = get("boxContents");

	int divider = gtk_paned_get_position(GTK_PANED(panelMainContents));
	bool sidebarRight = control->getSettings()->isSidebarOnRight();
	if (sidebarRight == (gtk_paned_get_child2(GTK_PANED(panelMainContents)) == sidebar))
	{
		// Already correct
		return;
	}
	else
	{
		GtkAllocation allocation;
		gtk_widget_get_allocation(panelMainContents, &allocation);
		divider = allocation.width - divider;
	}

	g_object_ref(sidebar);
	g_object_ref(boxContents);

	gtk_container_remove(GTK_CONTAINER(panelMainContents), sidebar);
	gtk_container_remove(GTK_CONTAINER(panelMainContents), boxContents);

	if (sidebarRight)
	{
		gtk_paned_pack1(GTK_PANED(panelMainContents), boxContents, TRUE, FALSE);
		gtk_paned_pack2(GTK_PANED(panelMainContents), sidebar, FALSE, FALSE);
	}
	else
	{
		gtk_paned_pack1(GTK_PANED(panelMainContents), sidebar, FALSE, FALSE);
		gtk_paned_pack2(GTK_PANED(panelMainContents), boxContents, TRUE, FALSE);
	}

	gtk_paned_set_position(GTK_PANED(panelMainContents), divider);
	g_object_unref(sidebar);
	g_object_unref(boxContents);
}

void MainWindow::buttonCloseSidebarClicked(GtkButton* button, MainWindow* win)
{
	XOJ_CHECK_TYPE_OBJ(win, MainWindow);

	win->setSidebarVisible(false);
}

bool MainWindow::onKeyPressCallback(GtkWidget* widget, GdkEventKey* event, MainWindow* win)
{

	if (win->getXournal()->getSelection())
	{
		//something is selected - give that control
		return false;
	}
	else if (win->getXournal()->getTextEditor())
	{
		//editing text - give that control
		return false;
	}
	else if (event->keyval == GDK_KEY_Escape)
	{
		win->getControl()->getSearchBar()->showSearchBar(false);
		return true;
	}
	else
	{
		return false;
	}
}

bool MainWindow::deleteEventCallback(GtkWidget* widget, GdkEvent* event, Control* control)
{
	control->quit();

	return true;
}

void MainWindow::setSidebarVisible(bool visible)
{
	XOJ_CHECK_TYPE(MainWindow);

	Settings* settings = control->getSettings();
	GtkWidget* sidebar = get("sidebar");
	GtkWidget* panel = get("panelMainContents");

	gtk_widget_set_visible(sidebar, visible);
	settings->setSidebarVisible(visible);

	if(!visible && (control->getSidebar() != NULL))
	{
		saveSidebarSize();
	}

	if(visible)
	{
		gtk_paned_set_position(GTK_PANED(panel), settings->getSidebarWidth());
	}

	GtkWidget* w = get("menuViewSidebarVisible");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(w), visible);
}

void MainWindow::saveSidebarSize()
{
	XOJ_CHECK_TYPE(MainWindow);

	GtkWidget* panel = get("panelMainContents");

	this->control->getSettings()->setSidebarWidth(gtk_paned_get_position(GTK_PANED(panel)));
}

void MainWindow::setMaximized(bool maximized)
{
	XOJ_CHECK_TYPE(MainWindow);

	this->maximized = maximized;
}

bool MainWindow::isMaximized()
{
	XOJ_CHECK_TYPE(MainWindow);

	return this->maximized;
}

XournalView* MainWindow::getXournal()
{
	XOJ_CHECK_TYPE(MainWindow);

	return xournal;
}

bool MainWindow::windowStateEventCallback(GtkWidget* window, GdkEventWindowState* event, MainWindow* win)
{
	XOJ_CHECK_TYPE_OBJ(win, MainWindow);
	win->setMaximized(gtk_window_is_maximized(GTK_WINDOW(window)));

	return false;
}

void MainWindow::reloadToolbars()
{
	XOJ_CHECK_TYPE(MainWindow);

	bool inDragAndDrop = this->control->isInDragAndDropToolbar();

	ToolbarData* d = getSelectedToolbar();

	if (inDragAndDrop)
	{
		this->control->endDragDropToolbar();
	}

	this->clearToolbar();
	this->toolbarSelected(d);

	if (inDragAndDrop)
	{
		this->control->startDragDropToolbar();
	}
}

void MainWindow::toolbarSelected(ToolbarData* d)
{
	XOJ_CHECK_TYPE(MainWindow);

	if (!this->toolbarIntialized || this->selectedToolbar == d)
	{
		return;
	}

	Settings* settings = control->getSettings();
	settings->setSelectedToolbar(d->getId());

	this->clearToolbar();
	this->loadToolbar(d);
}

ToolbarData* MainWindow::clearToolbar()
{
	XOJ_CHECK_TYPE(MainWindow);

	if (this->selectedToolbar != NULL)
	{
		for (int i = 0; i < TOOLBAR_DEFINITIONS_LEN; i++)
		{
			this->toolbar->unloadToolbar(this->toolbarWidgets[i]);
		}

		this->toolbar->freeDynamicToolbarItems();
	}

	ToolbarData* oldData = this->selectedToolbar;

	this->selectedToolbar = NULL;

	return oldData;
}

void MainWindow::loadToolbar(ToolbarData* d)
{
	XOJ_CHECK_TYPE(MainWindow);

	this->selectedToolbar = d;

	for (int i = 0; i < TOOLBAR_DEFINITIONS_LEN; i++)
	{
		this->toolbar->load(d, this->toolbarWidgets[i], TOOLBAR_DEFINITIONS[i].propName, TOOLBAR_DEFINITIONS[i].horizontal);
	}
	
	this->floatingToolbox->flagRecalculateSizeRequired();	
}

ToolbarData* MainWindow::getSelectedToolbar()
{
	XOJ_CHECK_TYPE(MainWindow);

	return this->selectedToolbar;
}

GtkWidget** MainWindow::getToolbarWidgets(int& length)
{
	XOJ_CHECK_TYPE(MainWindow);

	length = TOOLBAR_DEFINITIONS_LEN;
	return this->toolbarWidgets;
}

const char* MainWindow::getToolbarName(GtkToolbar* toolbar)
{
	XOJ_CHECK_TYPE(MainWindow);

	for (int i = 0; i < TOOLBAR_DEFINITIONS_LEN; i++)
	{
		if ((void*) this->toolbarWidgets[i] == (void*) toolbar)
		{
			return TOOLBAR_DEFINITIONS[i].propName;
		}
	}

	return "";
}

void MainWindow::setControlTmpDisabled(bool disabled)
{
	XOJ_CHECK_TYPE(MainWindow);

	toolbar->setTmpDisabled(disabled);
	toolbarSelectMenu->setTmpDisabled(disabled);

	GtkWidget* menuFileRecent = get("menuFileRecent");
	gtk_widget_set_sensitive(menuFileRecent, !disabled);
}

void MainWindow::updateToolbarMenu()
{
	XOJ_CHECK_TYPE(MainWindow);

	createToolbarAndMenu();
}

void MainWindow::createToolbarAndMenu()
{
	XOJ_CHECK_TYPE(MainWindow);

	GtkMenuShell* menubar = GTK_MENU_SHELL(get("menuViewToolbar"));
	g_return_if_fail(menubar != NULL);

	toolbarSelectMenu->updateToolbarMenu(menubar, control->getSettings(), toolbar);

	ToolbarData* td = toolbarSelectMenu->getSelectedToolbar();
	if (td)
	{
		this->toolbarIntialized = true;
		toolbarSelected(td);
	}

	if (!this->control->getAudioController()->isPlaying())
	{
		this->getToolMenuHandler()->disableAudioPlaybackButtons();
	}

	this->control->getScheduler()->unblockRerenderZoom();
}

void MainWindow::setFontButtonFont(XojFont& font)
{
	XOJ_CHECK_TYPE(MainWindow);

	toolbar->setFontButtonFont(font);
}

XojFont MainWindow::getFontButtonFont()
{
	XOJ_CHECK_TYPE(MainWindow);

	return toolbar->getFontButtonFont();
}

void MainWindow::updatePageNumbers(size_t page, size_t pagecount, size_t pdfpage)
{
	XOJ_CHECK_TYPE(MainWindow);

	SpinPageAdapter* spinPageNo = getSpinPageNo();

	size_t min;
	size_t max = pagecount;

	if (pagecount == 0)
	{
		min = 0;
		page = 0;
	}
	else
	{
		min = 1;
		page++;
	}

	spinPageNo->setMinMaxPage(min, max);
	spinPageNo->setPage(page);

	string pdfText;
	if (pdfpage != size_t_npos)
	{
		pdfText = string(", ") + FS(_F("PDF Page {1}") % (pdfpage + 1));
	}
	toolbar->setPageText(FS(C_F("Page {pagenumber} \"of {pagecount}\"", " of {1}{2}") % pagecount % pdfText));
}

void MainWindow::rebuildLayerMenu()
{
	XOJ_CHECK_TYPE(MainWindow);

	layerVisibilityChanged();
}

void MainWindow::layerVisibilityChanged()
{
	XOJ_CHECK_TYPE(MainWindow);

	LayerController* lc = control->getLayerController();

	int layer = lc->getCurrentLayerId();
	int maxLayer = lc->getLayerCount();

	control->fireEnableAction(ACTION_DELETE_LAYER, layer > 0);
	control->fireEnableAction(ACTION_GOTO_NEXT_LAYER, layer < maxLayer);
	control->fireEnableAction(ACTION_GOTO_PREVIOUS_LAYER, layer > 0);
	control->fireEnableAction(ACTION_GOTO_TOP_LAYER, layer < maxLayer);
}

void MainWindow::setRecentMenu(GtkWidget* submenu)
{
	XOJ_CHECK_TYPE(MainWindow);

	GtkWidget* menuitem = get("menuFileRecent");
	g_return_if_fail(menuitem != NULL);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem), submenu);
}

void MainWindow::show(GtkWindow* parent)
{
	XOJ_CHECK_TYPE(MainWindow);

	gtk_widget_show(this->window);
}

void MainWindow::setUndoDescription(string description)
{
	XOJ_CHECK_TYPE(MainWindow);

	toolbar->setUndoDescription(description);
}

void MainWindow::setRedoDescription(string description)
{
	XOJ_CHECK_TYPE(MainWindow);

	toolbar->setRedoDescription(description);
}

SpinPageAdapter* MainWindow::getSpinPageNo()
{
	XOJ_CHECK_TYPE(MainWindow);

	return toolbar->getPageSpinner();
}

ToolbarModel* MainWindow::getToolbarModel()
{
	XOJ_CHECK_TYPE(MainWindow);

	return this->toolbar->getModel();
}

ToolMenuHandler* MainWindow::getToolMenuHandler()
{
	XOJ_CHECK_TYPE(MainWindow);

	return this->toolbar;
}

void MainWindow::disableAudioPlaybackButtons()
{
	XOJ_CHECK_TYPE(MainWindow);

	setAudioPlaybackPaused(false);

	this->getToolMenuHandler()->disableAudioPlaybackButtons();
}

void MainWindow::enableAudioPlaybackButtons()
{
	XOJ_CHECK_TYPE(MainWindow);

	this->getToolMenuHandler()->enableAudioPlaybackButtons();
}

void MainWindow::setAudioPlaybackPaused(bool paused)
{
	XOJ_CHECK_TYPE(MainWindow);

	this->getToolMenuHandler()->setAudioPlaybackPaused(paused);
}

