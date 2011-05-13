#include "../control/Control.h"
#include "MainWindow.h"
#include "XournalView.h"
#include "../cfg.h"
#include "toolbarMenubar/ToolMenuHandler.h"
#include "../gui/GladeSearchpath.h"
#include "toolbarMenubar/model/ToolbarData.h"
#include "toolbarMenubar/model/ToolbarModel.h"

#include <config.h>
#include <glib/gi18n-lib.h>

typedef struct {
	const char * guiName;
	const char * propName;
	bool horizontal;
} ToolbarEntryDefintion;


const static ToolbarEntryDefintion TOOLBAR_DEFINITIONS[] = {
	{ "tbTop1", "toolbarTop1", true },
	{ "tbTop2", "toolbarTop2", true },
	{ "tbLeft1", "toolbarLeft1", false },
	{ "tbLeft2", "toolbarLeft2", false },
	{ "tbRight1", "toolbarRight1", false },
	{ "tbRight2", "toolbarRight2", false },
	{ "tbBottom1", "toolbarBottom1", true },
	{ "tbBottom2", "toolbarBottom2", true }
};

const static int TOOLBAR_DEFINITIONS_LEN = G_N_ELEMENTS(TOOLBAR_DEFINITIONS);

MainWindow::MainWindow(GladeSearchpath * gladeSearchPath, Control * control) :
	GladeGui(gladeSearchPath, "main.glade", "mainWindow") {

	XOJ_INIT_TYPE(MainWindow);

	this->control = control;
	this->toolbarIntialized = false;
	this->toolbarGroup = NULL;
	this->selectedToolbar = NULL;
	this->toolbarWidgets = new GtkWidget*[TOOLBAR_DEFINITIONS_LEN];

	for (int i = 0; i < TOOLBAR_DEFINITIONS_LEN; i++) {
		GtkWidget * w = get(TOOLBAR_DEFINITIONS[i].guiName);
		g_object_ref(w);
		this->toolbarWidgets[i] = w;
	}

	this->maximized = false;
	this->toolbarMenuData = NULL;
	this->toolbarMenuitems = NULL;

	this->toolbarSpacerItems = NULL;

	GtkRange * hrang = GTK_RANGE(get("scrollHorizontal"));
	GtkRange * vrang = GTK_RANGE(get("scrollVertical"));
	this->xournal = new XournalView(get("tableXournal"), hrang, vrang, control);

	setSidebarVisible(control->getSettings()->isSidebarVisible());

	// Window handler
	g_signal_connect(this->window, "delete-event", (GCallback) & deleteEventCallback, this->control);
	g_signal_connect(this->window, "window_state_event", G_CALLBACK(&windowStateEventCallback), this);

	g_signal_connect(get("buttonCloseSidebar"), "clicked", G_CALLBACK(buttonCloseSidebarClicked), this);

	this->toolbar = new ToolMenuHandler(this->control, this->control->getZoomControl(), this, this->control->getToolHandler());

	char * file = gladeSearchPath->findFile(NULL, "toolbar.ini");

	ToolbarModel * tbModel = this->toolbar->getModel();

	if (!tbModel->parse(file, true)) {
		GtkWidget* dlg = gtk_message_dialog_new(GTK_WINDOW(this->window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
				_("Could not parse general toolbar.ini file: %s\nNo Toolbars will be available"), file);

		gtk_dialog_run(GTK_DIALOG(dlg));
		gtk_widget_hide(dlg);
		gtk_widget_destroy(dlg);
	}

	g_free(file);

	file = g_build_filename(g_get_home_dir(), G_DIR_SEPARATOR_S, CONFIG_DIR, G_DIR_SEPARATOR_S, TOOLBAR_CONFIG, NULL);
	if (g_file_test(file, G_FILE_TEST_EXISTS)) {
		if (!tbModel->parse(file, false)) {
			GtkWidget* dlg = gtk_message_dialog_new(GTK_WINDOW(this->window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
					_("Could not parse custom toolbar.ini file: %s\nToolbars will not be available"), file);

			gtk_dialog_run(GTK_DIALOG(dlg));
			gtk_widget_hide(dlg);
			gtk_widget_destroy(dlg);
		}
	}
	g_free(file);

	initToolbarAndMenu();

	g_signal_connect(getSpinPageNo(), "value-changed", G_CALLBACK(pageNrSpinChangedCallback), this);

	GtkWidget * menuViewSidebarVisible = get("menuViewSidebarVisible");
	g_signal_connect(menuViewSidebarVisible, "toggled", (GCallback) viewShowSidebar, this);

	updateScrollbarSidebarPosition();

	gtk_window_set_default_size(GTK_WINDOW(this->window), control->getSettings()->getMainWndWidth(), control->getSettings()->getMainWndHeight());

	if (control->getSettings()->isMainWndMaximized()) {
		gtk_window_maximize(GTK_WINDOW(this->window));
	} else {
		gtk_window_unmaximize(GTK_WINDOW(this->window));
	}

	// Drag and Drop
	g_signal_connect(this->window, "drag-data-received", G_CALLBACK(dragDataRecived), this);

	gtk_drag_dest_set(this->window, GTK_DEST_DEFAULT_ALL, NULL, 0, GDK_ACTION_COPY);
	gtk_drag_dest_add_uri_targets(this->window);
	gtk_drag_dest_add_image_targets(this->window);
	gtk_drag_dest_add_text_targets(this->window);
}

class MenuSelectToolbarData {
public:
	MenuSelectToolbarData(MainWindow *win, GtkWidget * item, ToolbarData * d) {
		this->win = win;
		this->item = item;
		this->d = d;
	}

	MainWindow * win;
	GtkWidget * item;
	ToolbarData * d;
};

MainWindow::~MainWindow() {
	XOJ_CHECK_TYPE(MainWindow);

	for (GList * l = this->toolbarMenuData; l != NULL; l = l->next) {
		MenuSelectToolbarData * data = (MenuSelectToolbarData *) l->data;
		delete data;
	}

	for (int i = 0; i < TOOLBAR_DEFINITIONS_LEN; i++) {
		g_object_unref(this->toolbarWidgets[i]);
	}

	delete[] this->toolbarWidgets;
	this->toolbarWidgets = NULL;

	g_list_free(this->toolbarMenuData);
	this->toolbarMenuData = NULL;
	g_list_free(this->toolbarMenuitems);
	this->toolbarMenuitems = NULL;

	delete this->xournal;
	this->xournal = NULL;

	delete this->toolbar;
	this->toolbar = NULL;

	XOJ_RELEASE_TYPE(MainWindow);
}

bool cancellable_cancel(GCancellable * cancel) {
	g_cancellable_cancel(cancel);

	g_warning("Timeout... Cancel loading URL");

	return false;
}

void MainWindow::dragDataRecived(GtkWidget * widget, GdkDragContext * dragContext, gint x, gint y, GtkSelectionData * data,
		guint info, guint time, MainWindow * win) {

	GtkWidget * source = gtk_drag_get_source_widget(dragContext);
	if (source && widget == gtk_widget_get_toplevel(source)) {
		gtk_drag_finish(dragContext, false, false, time);
		return;
	}

	guchar * text = gtk_selection_data_get_text(data);
	if (text) {
		win->control->clipboardPasteText((const char *) text);

		g_free(text);
		gtk_drag_finish(dragContext, true, false, time);
		return;
	}

	GdkPixbuf * image = gtk_selection_data_get_pixbuf(data);
	if (image) {
		win->control->clipboardPasteImage(image);

		gdk_pixbuf_unref(image);
		gtk_drag_finish(dragContext, true, false, time);
		return;
	}

	// TODO LOW PRIO: use x and y for insert location!

	gchar ** uris = gtk_selection_data_get_uris(data);
	if (uris) {
		for (int i = 0; uris[i] != NULL && i < 3; i++) {
			const char * uri = uris[i];

			// TODO LOW PRIO: check first if its an image
//			GSList * imageFormats = gdk_pixbuf_get_formats();
//			for(GSList * l = imageFormats; l != NULL; l = l->next) {
//				GdkPixbufFormat * f = (GdkPixbufFormat *)l->data;
//				printf("", f);
//			}
//
//			g_slist_free(imageFormats);

			GFile * file = g_file_new_for_uri(uri);
			GError * err = NULL;
			GCancellable * cancel = g_cancellable_new();

			int cancelTimeout = g_timeout_add(3000, (GSourceFunc)cancellable_cancel, cancel);

			GFileInputStream * in = g_file_read(file, cancel, &err);

			if(g_cancellable_is_cancelled(cancel)) {
				continue;
			}

			g_object_unref(file);
			if (err == NULL) {
				GdkPixbuf * pixbuf = gdk_pixbuf_new_from_stream(G_INPUT_STREAM(in), cancel, NULL);
				if(g_cancellable_is_cancelled(cancel)) {
					continue;
				}
				g_input_stream_close(G_INPUT_STREAM(in), cancel, NULL);
				if(g_cancellable_is_cancelled(cancel)) {
					continue;
				}

				if (pixbuf) {
					win->control->clipboardPasteImage(pixbuf);

					gdk_pixbuf_unref(pixbuf);
				}
			} else {
				g_error_free(err);
			}

			if(!g_cancellable_is_cancelled(cancel)) {
				g_source_remove(cancelTimeout);
			}
			g_object_unref(cancel);

			//TODO LOW PRIO: handle .xoj, .pdf and Images
			printf("open uri: %s\n", uris[i]);
		}

		gtk_drag_finish(dragContext, true, false, time);

		g_strfreev(uris);
	}

	gtk_drag_finish(dragContext, false, false, time);
}

void MainWindow::viewShowSidebar(GtkCheckMenuItem * checkmenuitem, MainWindow * win) {
	XOJ_CHECK_TYPE_OBJ(win, MainWindow);

	bool a = gtk_check_menu_item_get_active(checkmenuitem);
	if (win->control->getSettings()->isSidebarVisible() == a) {
		return;
	}
	win->setSidebarVisible(a);
}

Control * MainWindow::getControl() {
	XOJ_CHECK_TYPE(MainWindow);

	return control;
}

void MainWindow::updateScrollbarSidebarPosition() {
	XOJ_CHECK_TYPE(MainWindow);

	GtkWidget * panelMainContents = get("panelMainContents");
	GtkWidget * sidebarContents = get("sidebarContents");
	GtkWidget * tableXournal = get("tableXournal");
	GtkWidget * scrollVertical = get("scrollVertical");

	g_object_ref(scrollVertical);
	bool scrollbarOnLeft = control->getSettings()->isScrollbarOnLeft();

	gtk_container_remove(GTK_CONTAINER(tableXournal), scrollVertical);
	if (scrollbarOnLeft) {
		gtk_table_attach(GTK_TABLE(tableXournal), scrollVertical, 0, 1, 0, 1, (GtkAttachOptions) 0, GTK_FILL, 0, 0);
	} else {
		gtk_table_attach(GTK_TABLE(tableXournal), scrollVertical, 2, 3, 0, 1, (GtkAttachOptions) 0, GTK_FILL, 0, 0);
	}
	Sidebar * sidebar = this->control->getSidebar();
	if (sidebar) {
		sidebar->setBackgroundWhite();
	}

	g_object_unref(scrollVertical);

	int divider = gtk_paned_get_position(GTK_PANED(panelMainContents));
	bool sidebarRight = control->getSettings()->isSidebarOnRight();
	if (sidebarRight == (gtk_paned_get_child2(GTK_PANED(panelMainContents)) == sidebarContents)) {
		// Already correct
		return;
	} else {
		GtkAllocation allocation;
		gtk_widget_get_allocation(panelMainContents, &allocation);
		divider = allocation.width - divider;
	}

	g_object_ref(sidebarContents);
	g_object_ref(tableXournal);

	gtk_container_remove(GTK_CONTAINER(panelMainContents), sidebarContents);
	gtk_container_remove(GTK_CONTAINER(panelMainContents), tableXournal);

	if (sidebarRight) {
		gtk_paned_pack1(GTK_PANED(panelMainContents), tableXournal, true, true);
		gtk_paned_pack2(GTK_PANED(panelMainContents), sidebarContents, false, true);
	} else {
		gtk_paned_pack1(GTK_PANED(panelMainContents), sidebarContents, false, true);
		gtk_paned_pack2(GTK_PANED(panelMainContents), tableXournal, true, true);
	}

	g_object_unref(sidebarContents);
	g_object_unref(tableXournal);
}

void MainWindow::buttonCloseSidebarClicked(GtkButton * button, MainWindow * win) {
	XOJ_CHECK_TYPE_OBJ(win, MainWindow);

	win->setSidebarVisible(false);
}

bool MainWindow::deleteEventCallback(GtkWidget * widget, GdkEvent * event, Control * control) {
	control->quit();

	return true;
}

void MainWindow::setSidebarVisible(bool visible) {
	XOJ_CHECK_TYPE(MainWindow);

	GtkWidget * sidebar = get("sidebarContents");
	gtk_widget_set_visible(sidebar, visible);
	control->getSettings()->setSidebarVisible(visible);

	GtkWidget * w = get("menuViewSidebarVisible");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(w), visible);
}

bool pageNrSpinChangedTimerCallback(Control * control) {
	gdk_threads_enter();
	control->getScrollHandler()->scrollToSpinPange();
	gdk_threads_leave();
	return false;
}

void MainWindow::pageNrSpinChangedCallback(GtkSpinButton * spinbutton, MainWindow * win) {
	XOJ_CHECK_TYPE_OBJ(win, MainWindow);

	static int lastId = 0;
	if (lastId) {
		g_source_remove(lastId);
	}

	// Give the spin button some time to realease, if we don't do he will send new events...
	lastId = g_timeout_add(100, (GSourceFunc) &pageNrSpinChangedTimerCallback, win->control);
}

void tbSelectMenuitemActivated(GtkMenuItem * menuitem, MenuSelectToolbarData * data) {
	data->win->toolbarSelected(data->d);
}

void MainWindow::setMaximized(bool maximized) {
	XOJ_CHECK_TYPE(MainWindow);

	this->maximized = maximized;
}

bool MainWindow::isMaximized() {
	XOJ_CHECK_TYPE(MainWindow);

	return this->maximized;
}

XournalView * MainWindow::getXournal() {
	XOJ_CHECK_TYPE(MainWindow);

	return xournal;
}

bool MainWindow::windowStateEventCallback(GtkWidget * window, GdkEventWindowState * event, MainWindow * win) {
	XOJ_CHECK_TYPE_OBJ(win, MainWindow);

	if (!(event->new_window_state & GDK_WINDOW_STATE_FULLSCREEN)) {
		gboolean maximized;

		maximized = event->new_window_state & GDK_WINDOW_STATE_MAXIMIZED;
		win->setMaximized(maximized);
	}

	return false;
}

void MainWindow::toolbarSelected(ToolbarData * d) {
	XOJ_CHECK_TYPE(MainWindow);

	if (!toolbarIntialized || this->selectedToolbar == d) {
		return;
	}

	Settings * settings = control->getSettings();
	settings->setSelectedToolbar(d->getId());

	if (selectedToolbar != NULL) {
		for (int i = 0; i < TOOLBAR_DEFINITIONS_LEN; i++) {
			this->toolbar->unloadToolbar(this->toolbarWidgets[i]);
		}

		this->toolbar->freeDynamicToolbarItems();
	}
	this->selectedToolbar = d;

	for (int i = 0; i < TOOLBAR_DEFINITIONS_LEN; i++) {
		this->toolbar->load(d, this->toolbarWidgets[i], TOOLBAR_DEFINITIONS[i].propName, TOOLBAR_DEFINITIONS[i].horizontal);
	}
}

GtkWidget ** MainWindow::getToolbarWidgets(int & length) {
	length = TOOLBAR_DEFINITIONS_LEN;
	return this->toolbarWidgets;
}

void MainWindow::startToolbarEditMode() {
	this->toolbar->startEditMode();
	for (int i = 0; i < TOOLBAR_DEFINITIONS_LEN; i++) {
		gtk_widget_show(this->toolbarWidgets[i]);

		GtkToolbar * tb = GTK_TOOLBAR(this->toolbarWidgets[i]);
		gtk_toolbar_set_icon_size(tb, GTK_ICON_SIZE_SMALL_TOOLBAR);

		GtkToolItem * it = gtk_tool_item_new();
		this->toolbarSpacerItems = g_list_append(this->toolbarSpacerItems, it);
		gtk_toolbar_insert(tb, it, 0);

		GtkOrientation orientation = gtk_toolbar_get_orientation(tb);
		if (orientation == GTK_ORIENTATION_HORIZONTAL) {
			GtkAllocation alloc = { 0, 0, 0, 20 };
			gtk_widget_set_allocation(GTK_WIDGET(it), &alloc);
		} else if (orientation == GTK_ORIENTATION_VERTICAL) {
			GtkAllocation alloc = { 0, 0, 20, 0 };
			gtk_widget_set_allocation(GTK_WIDGET(it), &alloc);
		}
	}
	// TODO implement startToolbarEditMode
}

void MainWindow::endToolbarEditMode() {
	this->toolbar->endEditMode();

	for (GList * l = this->toolbarSpacerItems; l != NULL; l = l->next) {
		GtkWidget * w = GTK_WIDGET(l->data);
		GtkWidget * parent = gtk_widget_get_parent(w);
		gtk_container_remove(GTK_CONTAINER(parent), w);
	}
	g_list_free(this->toolbarSpacerItems);
	this->toolbarSpacerItems = NULL;

	for (int i = 0; i < TOOLBAR_DEFINITIONS_LEN; i++) {
		GtkToolbar * tb = GTK_TOOLBAR(this->toolbarWidgets[i]);
		if (gtk_toolbar_get_n_items(tb) == 0) {
			gtk_widget_hide(GTK_WIDGET(tb));
		}
	}
}

void MainWindow::setControlTmpDisabled(bool disabled) {
	XOJ_CHECK_TYPE(MainWindow);

	toolbar->setTmpDisabled(disabled);

	for (GList * l = this->toolbarMenuData; l != NULL; l = l->next) {
		MenuSelectToolbarData * data = (MenuSelectToolbarData *) l->data;
		gtk_widget_set_sensitive(data->item, !disabled);
	}

	GtkWidget * menuFileRecent = get("menuFileRecent");
	gtk_widget_set_sensitive(menuFileRecent, !disabled);

	GtkWidget * w = get("cbSelectSidebar");
	gtk_widget_set_sensitive(w, !disabled);
}

void MainWindow::updateToolbarMenu() {
	XOJ_CHECK_TYPE(MainWindow);

	GtkContainer * menubar = GTK_CONTAINER(get("menuViewToolbar"));
	g_return_if_fail(menubar != NULL);

	for (GList * l = this->toolbarMenuitems; l != NULL; l = l->next) {
		GtkWidget * w = GTK_WIDGET(l->data);
		gtk_container_remove(menubar, w);
	}
	g_list_free(this->toolbarMenuitems);
	this->toolbarMenuitems = NULL;

	for (GList * l = this->toolbarMenuData; l != NULL; l = l->next) {
		MenuSelectToolbarData * data = (MenuSelectToolbarData *) l->data;
		delete data;
	}
	g_list_free(this->toolbarMenuData);
	this->toolbarMenuData = NULL;

	g_slist_free(this->toolbarGroup);
	this->toolbarGroup = NULL;

	initToolbarAndMenu();
}

void MainWindow::initToolbarAndMenu() {
	XOJ_CHECK_TYPE(MainWindow);

	GtkMenuShell * menubar = GTK_MENU_SHELL(get("menuViewToolbar"));
	g_return_if_fail(menubar != NULL);

	ListIterator<ToolbarData *> it = this->toolbar->getModel()->iterator();
	GtkWidget * item = NULL;
	GtkWidget * selectedItem = NULL;
	ToolbarData * selectedData = NULL;

	Settings * settings = control->getSettings();
	String selectedId = settings->getSelectedToolbar();

	bool predefined = true;
	int menuPos = 0;

	while (it.hasNext()) {
		ToolbarData * d = it.next();
		if (selectedData == NULL) {
			selectedData = d;
			selectedItem = item;
		}

		item = gtk_radio_menu_item_new_with_label(this->toolbarGroup, d->getName().c_str());
		this->toolbarGroup = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(item));

		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), false);

		MenuSelectToolbarData * data = new MenuSelectToolbarData(this, item, d);

		this->toolbarMenuData = g_list_append(this->toolbarMenuData, data);

		if (selectedId == d->getId()) {
			selectedData = d;
			selectedItem = item;
		}

		g_signal_connect(item, "activate", G_CALLBACK(tbSelectMenuitemActivated), data);

		gtk_widget_show(item);

		if (predefined && !d->isPredefined()) {
			GtkWidget * separator = gtk_separator_menu_item_new();
			gtk_widget_show(separator);
			gtk_menu_shell_insert(menubar, separator, menuPos++);

			predefined = false;
			this->toolbarMenuitems = g_list_append(this->toolbarMenuitems, separator);
		}

		gtk_menu_shell_insert(menubar, item, menuPos++);

		this->toolbarMenuitems = g_list_append(this->toolbarMenuitems, item);
	}

	if (selectedData) {
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(selectedItem), TRUE);
		this->toolbarIntialized = true;
		toolbarSelected(selectedData);
	}

	this->control->getScheduler()->unblockRerenderZoom();
}

int MainWindow::getCurrentLayer() {
	XOJ_CHECK_TYPE(MainWindow);

	return toolbar->getSelectedLayer();
}

void MainWindow::setFontButtonFont(XojFont & font) {
	XOJ_CHECK_TYPE(MainWindow);

	toolbar->setFontButtonFont(font);
}

XojFont MainWindow::getFontButtonFont() {
	XOJ_CHECK_TYPE(MainWindow);

	return toolbar->getFontButtonFont();
}

void MainWindow::updatePageNumbers(int page, int pagecount, int pdfpage) {
	XOJ_CHECK_TYPE(MainWindow);

	GtkWidget * spinPageNo = getSpinPageNo();

	int min = 1;
	int max = pagecount;

	if (pagecount == 0) {
		min = 0;
		page = 0;
	} else {
		page++;
	}

	gtk_spin_button_set_range(GTK_SPIN_BUTTON(spinPageNo), min, max);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinPageNo), page);

	char * pdfText = NULL;
	if (pdfpage < 0) {
		pdfText = g_strdup("");
	} else {
		pdfText = g_strdup_printf(_(", PDF Page %i"), pdfpage + 1);
	}

	String text = String::format(_("of %i%s"), pagecount, pdfText);
	toolbar->setPageText(text);
	g_free(pdfText);

	updateLayerCombobox();
}

void MainWindow::updateLayerCombobox() {
	XOJ_CHECK_TYPE(MainWindow);

	PageRef p = control->getCurrentPage();

	int layer = 0;

	if (p) {
		layer = p.getSelectedLayerId();
		toolbar->setLayerCount(p.getLayerCount(), layer);
	} else {
		toolbar->setLayerCount(-1, -1);
	}

	control->fireEnableAction(ACTION_DELETE_LAYER, layer > 0);
}

void MainWindow::setRecentMenu(GtkWidget * submenu) {
	XOJ_CHECK_TYPE(MainWindow);

	GtkWidget * menuitem = get("menuFileRecent");
	g_return_if_fail(menuitem != NULL);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem), submenu);
}

void MainWindow::show() {
	XOJ_CHECK_TYPE(MainWindow);

	gtk_widget_show(this->window);
}

void MainWindow::setUndoDescription(String description) {
	XOJ_CHECK_TYPE(MainWindow);

	toolbar->setUndoDescription(description);
}

void MainWindow::setRedoDescription(String description) {
	XOJ_CHECK_TYPE(MainWindow);

	toolbar->setRedoDescription(description);
}

GtkWidget * MainWindow::getSpinPageNo() {
	XOJ_CHECK_TYPE(MainWindow);

	return toolbar->getPageSpinner();
}

ToolbarModel * MainWindow::getToolbarModel() {
	XOJ_CHECK_TYPE(MainWindow);

	return this->toolbar->getModel();
}

ToolMenuHandler * MainWindow::getToolMenuHandler() {
	XOJ_CHECK_TYPE(MainWindow);

	return this->toolbar;
}
