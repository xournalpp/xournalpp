#include "../control/Control.h"
#include "MainWindow.h"
#include "../cfg.h"
#include "../gettext.h"

MainWindow::MainWindow(Control * control) :
	GladeGui("main.glade", "mainWindow") {
	this->control = control;
	this->toolbarIntialized = false;
	this->toolbarGroup = NULL;
	this->selectedToolbar = NULL;
	this->tbTop1 = get("tbTop1");
	this->tbTop2 = get("tbTop2");
	this->tbLeft1 = get("tbLeft1");
	this->tbLeft2 = get("tbLeft2");
	this->tbRight1 = get("tbRight1");
	this->tbRight2 = get("tbRight2");
	this->tbBottom1 = get("tbBottom1");
	this->tbBottom2 = get("tbBottom2");
	this->maximized = false;
	this->toolbarMenuitem = NULL;

	this->xournal = new XournalWidget(get("scrolledwindowMain"), control);

	setSidebarVisible(control->getSettings()->isSidebarVisible());

	// Window handler
	g_signal_connect(this->window, "delete-event", (GCallback)&deleteEventCallback, this->control);
	g_signal_connect (this->window, "window_state_event", G_CALLBACK(&windowStateEventCallback), this);

	g_signal_connect(get("buttonCloseSidebar"), "clicked", G_CALLBACK(buttonCloseSidebarClicked), this);

	this->toolbar = new ToolMenuHandler(this->control, this->control->getZoomControl(), this,
			this->control->getToolHandler());

	gchar * file = g_build_filename(g_get_home_dir(), G_DIR_SEPARATOR_S, CONFIG_DIR, G_DIR_SEPARATOR_S, TOOLBAR_CONFIG,
			NULL);
	if (!this->toolbar->parse(file) || this->toolbar->shouldRecreate()) {
		if (!this->toolbar->writeDefault(file) || !this->toolbar->parse(file)) {
			GtkWidget* dlg = gtk_message_dialog_new(GTK_WINDOW(this->window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
					GTK_BUTTONS_OK,
					_("Could not create toolbar config file: %s\nCheck write permission, no Toolbars will be available"),
					file);

			gtk_dialog_run(GTK_DIALOG(dlg));
			gtk_widget_hide(dlg);
			gtk_widget_destroy(dlg);
		}
	}
	g_free(file);

	initToolbar();

	g_signal_connect(getSpinPageNo(), "value-changed", G_CALLBACK(pageNrSpinChangedCallback), this);

	GtkWidget * menuViewSidebarVisible = get("menuViewSidebarVisible");
	g_signal_connect(menuViewSidebarVisible, "toggled", (GCallback)viewShowSidebar, this);

	updateScrollbarSidebarPosition();

	gtk_window_set_default_size(GTK_WINDOW(window), control->getSettings()->getMainWndWidth(),
			control->getSettings()->getMainWndHeight());

	if (control->getSettings()->isMainWndMaximized()) {
		gtk_window_maximize(GTK_WINDOW(window));
	} else {
		gtk_window_unmaximize(GTK_WINDOW (window));
	}
}

class MenuSelectToolbarData {
public:
	MenuSelectToolbarData(MainWindow *win, GtkWidget * item, ToolbarData * d) {
		this->win = win;
		this->item = item;
		this->d = d;
	}

	MainWindow *win;
	GtkWidget * item;
	ToolbarData * d;
};

MainWindow::~MainWindow() {
	for (GList * l = this->toolbarMenuitem; l != NULL; l = l->next) {
		MenuSelectToolbarData * data = (MenuSelectToolbarData *) l->data;
		delete data;
	}

	g_list_free(this->toolbarMenuitem);
}

void MainWindow::viewShowSidebar(GtkCheckMenuItem *checkmenuitem, MainWindow * win) {
	bool a = gtk_check_menu_item_get_active(checkmenuitem);
	if (win->control->getSettings()->isSidebarVisible() == a) {
		return;
	}
	win->setSidebarVisible(a);
}

Control * MainWindow::getControl() {
	return control;
}

void MainWindow::updateScrollbarSidebarPosition() {
	GtkWidget * panelMainContents = get("panelMainContents");
	GtkWidget * sidebarContents = get("sidebarContents");
	GtkWidget * scrolledwindowMain = get("scrolledwindowMain");

	gtk_scrolled_window_set_placement(GTK_SCROLLED_WINDOW(scrolledwindowMain),
			control->getSettings()->isScrollbarOnLeft() ? GTK_CORNER_TOP_RIGHT : GTK_CORNER_TOP_LEFT);

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
	g_object_ref(scrolledwindowMain);

	gtk_container_remove(GTK_CONTAINER(panelMainContents), sidebarContents);
	gtk_container_remove(GTK_CONTAINER(panelMainContents), scrolledwindowMain);

	if (sidebarRight) {
		gtk_paned_pack1(GTK_PANED(panelMainContents), scrolledwindowMain, true, true);
		gtk_paned_pack2(GTK_PANED(panelMainContents), sidebarContents, false, true);
	} else {
		gtk_paned_pack1(GTK_PANED(panelMainContents), sidebarContents, false, true);
		gtk_paned_pack2(GTK_PANED(panelMainContents), scrolledwindowMain, true, true);
	}

	xournal->updateBackground();

	g_object_unref(sidebarContents);
	g_object_unref(scrolledwindowMain);
}

void MainWindow::buttonCloseSidebarClicked(GtkButton *button, MainWindow * win) {
	win->setSidebarVisible(false);
}

bool MainWindow::deleteEventCallback(GtkWidget *widget, GdkEvent *event, Control * control) {
	control->quit();

	return true;
}

void MainWindow::setSidebarVisible(bool visible) {
	GtkWidget * sidebar = get("sidebarContents");
	gtk_widget_set_visible(sidebar, visible);
	control->getSettings()->setSidebarVisible(visible);

	GtkWidget * w = get("menuViewSidebarVisible");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(w), visible);
}

bool pageNrSpinChangedTimerCallback(Control * control) {
	control->scrollToSpinPange();
	return false;
}

void MainWindow::pageNrSpinChangedCallback(GtkSpinButton *spinbutton, MainWindow * win) {
	static int lastId = 0;
	if (lastId) {
		g_source_remove(lastId);
	}

	// Give the spin button some time to realease, if we don't do he will send new events...
	lastId = g_timeout_add(100, (GSourceFunc) &pageNrSpinChangedTimerCallback, win->control);
}

void tbSelectMenuitemActivated(GtkMenuItem *menuitem, MenuSelectToolbarData * data) {
	data->win->toolbarSelected(data->d);
}

void MainWindow::setMaximized(bool maximized) {
	this->maximized = maximized;
}

bool MainWindow::isMaximized() {
	return this->maximized;
}

XournalWidget * MainWindow::getXournal() {
	return xournal;
}

bool MainWindow::windowStateEventCallback(GtkWidget *window, GdkEventWindowState *event, MainWindow * win) {
	if (!(event->new_window_state & GDK_WINDOW_STATE_FULLSCREEN)) {
		gboolean maximized;

		maximized = event->new_window_state & GDK_WINDOW_STATE_MAXIMIZED;
		win->setMaximized(maximized);
	}

	return false;
}

void MainWindow::toolbarSelected(ToolbarData * d) {
	if (!toolbarIntialized || this->selectedToolbar == d) {
		return;
	}

	Settings * settings = control->getSettings();
	settings->setSelectedToolbar(d->getId());

	if (selectedToolbar != NULL) {
		this->toolbar->unloadToolbar(this->tbTop1);
		this->toolbar->unloadToolbar(this->tbTop2);
		this->toolbar->unloadToolbar(this->tbLeft1);
		this->toolbar->unloadToolbar(this->tbLeft2);
		this->toolbar->unloadToolbar(this->tbRight1);
		this->toolbar->unloadToolbar(this->tbRight2);
		this->toolbar->unloadToolbar(this->tbBottom1);
		this->toolbar->unloadToolbar(this->tbBottom2);

		this->toolbar->freeToolbar();
	}
	this->selectedToolbar = d;

	this->toolbar->load(d, this->tbTop1, "toolbarTop1", true);
	this->toolbar->load(d, this->tbTop2, "toolbarTop2", true);
	this->toolbar->load(d, this->tbLeft1, "toolbarLeft1", false);
	this->toolbar->load(d, this->tbLeft2, "toolbarLeft2", false);
	this->toolbar->load(d, this->tbRight1, "toolbarRight1", false);
	this->toolbar->load(d, this->tbRight2, "toolbarRight2", false);
	this->toolbar->load(d, this->tbBottom1, "toolbarBottom1", true);
	this->toolbar->load(d, this->tbBottom2, "toolbarBottom2", true);
}

void MainWindow::setControlTmpDisabled(bool disabled) {
	toolbar->setTmpDisabled(disabled);

	for (GList * l = this->toolbarMenuitem; l != NULL; l = l->next) {
		MenuSelectToolbarData * data = (MenuSelectToolbarData *) l->data;
		gtk_widget_set_sensitive(data->item, !disabled);
	}

	GtkWidget * menuFileRecent = get("menuFileRecent");
	gtk_widget_set_sensitive(menuFileRecent, !disabled);

	GtkWidget * w = get("cbSelectSidebar");
	gtk_widget_set_sensitive(w, !disabled);

	// TODO: this->sidebar setTmpDisabled
}

void MainWindow::initToolbar() {
	GtkMenuShell * menubar = GTK_MENU_SHELL(get("menuViewToolbar"));
	g_return_if_fail(menubar != NULL);

	ListIterator<ToolbarData *> it = this->toolbar->iterator();
	GtkWidget * item = NULL;
	GtkWidget * selectedItem = NULL;
	ToolbarData * d = NULL;
	ToolbarData * selectedData = NULL;

	Settings * settings = control->getSettings();
	String selectedId = settings->getSelectedToolbar();

	while (it.hasNext()) {
		d = it.next();
		if (selectedData == NULL) {
			selectedData = d;
			selectedItem = item;
		}

		item = gtk_radio_menu_item_new_with_label(this->toolbarGroup, d->getName().c_str());
		this->toolbarGroup = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM (item));

		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), false);

		MenuSelectToolbarData * data = new MenuSelectToolbarData(this, item, d);

		this->toolbarMenuitem = g_list_append(this->toolbarMenuitem, data);

		if (selectedId == d->getId()) {
			selectedData = d;
			selectedItem = item;
		}

		g_signal_connect (item, "activate", G_CALLBACK (tbSelectMenuitemActivated), data);

		gtk_widget_show(item);
		gtk_menu_shell_append(menubar, item);
		gtk_widget_unrealize(item);
	}

	if (selectedData) {
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM (selectedItem), TRUE);
		this->toolbarIntialized = true;
		toolbarSelected(selectedData);
	}
}

MainWindow::operator GdkWindow *() {
	return GTK_WIDGET(getWindow())->window;
}

MainWindow::operator GtkWindow *() {
	return GTK_WINDOW(getWindow());
}

int MainWindow::getCurrentLayer() {
	return toolbar->getSelectedLayer();
}

void MainWindow::updatePageNumbers(int page, int pagecount, int pdfpage) {
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

	String text(g_strdup_printf(_("of %i%s"), pagecount, pdfText), true);
	toolbar->setPageText(text);
	g_free(pdfText);

	updateLayerCombobox();
}

void MainWindow::updateLayerCombobox() {
	XojPage * p = control->getCurrentPage();

	int layer = 0;

	if (p) {
		layer = p->getSelectedLayerId();
		toolbar->setLayerCount(p->getLayerCount(), layer);
	} else {
		toolbar->setLayerCount(-1, -1);
	}

	control->fireEnableAction(ACTION_DELETE_LAYER, layer > 0);
}

void MainWindow::setRecentMenu(GtkWidget * submenu) {
	GtkWidget * menuitem = get("menuFileRecent");
	g_return_if_fail(menuitem != NULL);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem), submenu);
}

void MainWindow::show() {
	gtk_widget_show(this->window);
	GtkWidget * widget = xournal->getWidget();
	gdk_window_set_background(GTK_LAYOUT(widget)->bin_window, &widget->style->dark[GTK_STATE_NORMAL]);
}

void MainWindow::setUndoDescription(String description) {
	toolbar->setUndoDescription(description);
}

void MainWindow::setRedoDescription(String description) {
	toolbar->setRedoDescription(description);
}

GtkWidget * MainWindow::getSpinPageNo() {
	return toolbar->getPageSpinner();
}
