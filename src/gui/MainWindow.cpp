#include "MainWindow.h"

#include "Layout.h"
#include "MainWindowToolbarMenu.h"
#include "ToolitemDragDrop.h"
#include "XournalView.h"

#include "control/Control.h"
#include "gui/GladeSearchpath.h"
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

#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>

#include <gdk/gdk.h>

#include <iostream>
using std::cout;
using std::endl;
namespace bf = boost::filesystem;

//those two have to be moved somewhere else in the future
gint sttime = 0;	
string audioFilename = "";
string audioFolder = "";
//gchar* audioFilename;

MainWindow::MainWindow(GladeSearchpath* gladeSearchPath, Control* control) :
		GladeGui(gladeSearchPath, "main.glade", "mainWindow")
{
	XOJ_INIT_TYPE(MainWindow);

	this->control = control;
	this->toolbarIntialized = false;
	this->selectedToolbar = NULL;
	this->toolbarWidgets = new GtkWidget*[TOOLBAR_DEFINITIONS_LEN];
	this->toolbarSelectMenu = new MainWindowToolbarMenu(this);

	for (int i = 0; i < TOOLBAR_DEFINITIONS_LEN; i++)
	{
		GtkWidget* w = get(TOOLBAR_DEFINITIONS[i].guiName);
		g_object_ref(w);
		this->toolbarWidgets[i] = w;
	}

	this->maximized = false;

#ifndef ENABLE_MATHTEX
	// if mathetex is disable disabled hide the menu entry
	gtk_widget_destroy(get("menuEditTex"));
#endif

	GtkWidget* vpXournal = get("vpXournal");

	this->xournal = new XournalView(vpXournal, control);

	setSidebarVisible(control->getSettings()->isSidebarVisible());

	// Window handler
	g_signal_connect(this->window, "delete-event", (GCallback) & deleteEventCallback, this->control);
	g_signal_connect(this->window, "window_state_event", G_CALLBACK(&windowStateEventCallback), this);

	g_signal_connect(get("buttonCloseSidebar"), "clicked", G_CALLBACK(buttonCloseSidebarClicked), this);

	//"watch over" all events
	g_signal_connect(this->window, "key-press-event", (GCallback) & onKeyPressCallback, this);

	this->toolbar = new ToolMenuHandler(this->control, this->control->getZoomControl(), this,
										this->control->getToolHandler(), GTK_WINDOW(getWindow()));

	string file = gladeSearchPath->findFile("", "toolbar.ini");

	ToolbarModel* tbModel = this->toolbar->getModel();

	if (!tbModel->parse(file, true))
	{
		GtkWidget* dlg = gtk_message_dialog_new(GTK_WINDOW(this->window),
												GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s",
												FC(_F("Could not parse general toolbar.ini file: {1}\n"
													  "No Toolbars will be available") % file));

		gtk_window_set_transient_for(GTK_WINDOW(dlg), GTK_WINDOW(getWindow()));
		gtk_dialog_run(GTK_DIALOG(dlg));
		gtk_widget_hide(dlg);
		gtk_widget_destroy(dlg);
	}

	file = string(g_get_home_dir()) + G_DIR_SEPARATOR_S + CONFIG_DIR + G_DIR_SEPARATOR_S + TOOLBAR_CONFIG;
	if (bf::exists(bf::path(file)))
	{
		if (!tbModel->parse(file, false))
		{
			GtkWidget* dlg = gtk_message_dialog_new(GTK_WINDOW(this->window),
													GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s",
													FC(_F("Could not parse custom toolbar.ini file: {1}\n"
														  "Toolbars will not be available") % file));

			gtk_window_set_transient_for(GTK_WINDOW(dlg), GTK_WINDOW(this->control->getWindow()->getWindow()));
			gtk_dialog_run(GTK_DIALOG(dlg));
			gtk_widget_hide(dlg);
			gtk_widget_destroy(dlg);
		}
	}

	createToolbarAndMenu();

	GtkWidget* menuViewSidebarVisible = get("menuViewSidebarVisible");
	g_signal_connect(menuViewSidebarVisible, "toggled", (GCallback) viewShowSidebar, this);

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

	// Drag and Drop
	g_signal_connect(this->window, "drag-data-received", G_CALLBACK(dragDataRecived), this);

	gtk_drag_dest_set(this->window, GTK_DEST_DEFAULT_ALL, NULL, 0, GDK_ACTION_COPY);
	gtk_drag_dest_add_uri_targets(this->window);
	gtk_drag_dest_add_image_targets(this->window);
	gtk_drag_dest_add_text_targets(this->window);
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

	delete this->xournal;
	this->xournal = NULL;

	delete this->toolbar;
	this->toolbar = NULL;

	XOJ_RELEASE_TYPE(MainWindow);
}

Layout* MainWindow::getLayout()
{
	XOJ_CHECK_TYPE(MainWindow);

	Layout* layout = gtk_xournal_get_layout(GTK_WIDGET(this->xournal->getWidget()));
	return layout;
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

	// TODO LOW PRIO: use x and y for insert location!

	gchar** uris = gtk_selection_data_get_uris(data);
	if (uris)
	{
		for (int i = 0; uris[i] != NULL && i < 3; i++)
		{
			const char* uri = uris[i];

			// TODO LOW PRIO: check first if its an image
			//			GSList * imageFormats = gdk_pixbuf_get_formats();
			//			for(GSList * l = imageFormats; l != NULL; l = l->next) {
			//				GdkPixbufFormat * f = (GdkPixbufFormat *)l->data;
			//				printf("", f);
			//			}
			//
			//			g_slist_free(imageFormats);

			GFile* file = g_file_new_for_uri(uri);
			GError* err = NULL;
			GCancellable* cancel = g_cancellable_new();

			int cancelTimeout = g_timeout_add(3000, (GSourceFunc) cancellable_cancel, cancel);

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

			//TODO LOW PRIO: handle .xoj, .pdf and Images
			cout << _F("Open URI: {1}") % uris[i] << endl;
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
	GtkWidget* sidebar = get("sidebar");
	GtkWidget* winXournal = get("winXournal");
	GtkScrolledWindow* scrolledWindow = GTK_SCROLLED_WINDOW(winXournal);

	bool scrollbarOnLeft = control->getSettings()->isScrollbarOnLeft();

	ScrollbarHideType type =
	    this->getControl()->getSettings()->getScrollbarHideType();

	if (scrollbarOnLeft)
	{
		gtk_scrolled_window_set_placement(scrolledWindow, GTK_CORNER_TOP_RIGHT);
	}
	else
	{
		gtk_scrolled_window_set_placement(scrolledWindow, GTK_CORNER_TOP_LEFT);
	}

	gtk_widget_set_visible(gtk_scrolled_window_get_hscrollbar(scrolledWindow),
	                       !(type & SCROLLBAR_HIDE_HORIZONTAL));

	gtk_widget_set_visible(gtk_scrolled_window_get_vscrollbar(scrolledWindow),
	                       !(type & SCROLLBAR_HIDE_VERTICAL));

	int divider = gtk_paned_get_position(GTK_PANED(panelMainContents));
	bool sidebarRight = control->getSettings()->isSidebarOnRight();
	if (sidebarRight == (gtk_paned_get_child2(GTK_PANED(panelMainContents)) ==
	                     sidebar))
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

	gtk_container_remove(GTK_CONTAINER(panelMainContents), sidebar);
	gtk_container_remove(GTK_CONTAINER(panelMainContents), winXournal);

	if (sidebarRight)
	{
		gtk_paned_pack1(GTK_PANED(panelMainContents), winXournal, TRUE, FALSE);
		gtk_paned_pack2(GTK_PANED(panelMainContents), sidebar, FALSE, FALSE);
	}
	else
	{
		gtk_paned_pack1(GTK_PANED(panelMainContents), sidebar, FALSE, FALSE);
		gtk_paned_pack2(GTK_PANED(panelMainContents), winXournal, TRUE, FALSE);
	}

	gtk_paned_set_position(GTK_PANED(panelMainContents), divider);
	g_object_unref(sidebar);
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
	else if (event->keyval == GDK_KEY_Down)
	{
		if (win->getControl()->getSettings()->isPresentationMode())
		{
			win->getControl()->getScrollHandler()->goToNextPage();
			return true;
		}
		else
		{
			win->getLayout()->scrollRelativ(0, 30);
			return true;
		}
	}
	else if (event->keyval == GDK_KEY_Up)
	{
		if (win->getControl()->getSettings()->isPresentationMode())
		{
			win->getControl()->getScrollHandler()->goToPreviousPage();
			return true;
		}
		else
		{
			win->getLayout()->scrollRelativ(0, -30);
			return true;
		}
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

	this->control->getScheduler()->unblockRerenderZoom();
}

int MainWindow::getCurrentLayer()
{
	XOJ_CHECK_TYPE(MainWindow);

	return toolbar->getSelectedLayer();
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

	updateLayerCombobox();
}

void MainWindow::updateLayerCombobox()
{
	XOJ_CHECK_TYPE(MainWindow);

	PageRef p = control->getCurrentPage();

	int layer = 0;
	int maxLayer = 0;

	if (p)
	{
		layer = p->getSelectedLayerId();
		maxLayer = p->getLayerCount();
		toolbar->setLayerCount(maxLayer, layer);
	}
	else
	{
		toolbar->setLayerCount(-1, -1);
	}

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
