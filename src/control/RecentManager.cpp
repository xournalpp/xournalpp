#include "RecentManager.h"

#include <config.h>
#include <i18n.h>

#include <gtk/gtk.h>

#include <boost/filesystem.hpp>
using boost::filesystem::path;

#include <iostream>
using std::cout;
using std::endl;

#define MIME "application/x-xoj"
#define MIME_PDF "application/x-pdf"
#define GROUP "xournal++"

RecentManager::RecentManager()
{
	XOJ_INIT_TYPE(RecentManager);

	this->maxRecent = 10;
	this->menu = gtk_menu_new();

	GtkRecentManager* recentManager = gtk_recent_manager_get_default();
	this->recentHandlerId = g_signal_connect(recentManager, "changed", G_CALLBACK(recentManagerChangedCallback), this);

	gdk_threads_enter();
	updateMenu();
	gdk_threads_leave();
}

RecentManager::~RecentManager()
{
	XOJ_CHECK_TYPE(RecentManager);

	if (this->recentHandlerId)
	{
		GtkRecentManager* recentManager = gtk_recent_manager_get_default();
		g_signal_handler_disconnect(recentManager, this->recentHandlerId);
		this->recentHandlerId = 0;
	}
	this->menu = NULL;

	XOJ_RELEASE_TYPE(RecentManager);
}

void RecentManager::addListener(RecentManagerListener* listener)
{
	XOJ_CHECK_TYPE(RecentManager);

	this->listener.push_back(listener);
}

void RecentManager::recentManagerChangedCallback(GtkRecentManager* manager, RecentManager* recentManager)
{
	// regenerate the menu when the model changes
	recentManager->updateMenu();
}

void RecentManager::addRecentFileFilename(path filename)
{
	XOJ_CHECK_TYPE(RecentManager);

	cout << bl::format("addRecentFileFilename: {1}") % filename << endl;

	GtkRecentManager* recentManager;
	GtkRecentData* recentData;

	static gchar * groups[2] = {g_strdup(GROUP), NULL};

	recentManager = gtk_recent_manager_get_default();

	recentData = g_slice_new(GtkRecentData);

	recentData->display_name = NULL;
	recentData->description = NULL;

	if (filename.extension() == ".pdf")
	{
		recentData->mime_type = (gchar*) g_strdup(MIME_PDF);
	}
	else
	{
		recentData->mime_type = (gchar*) g_strdup(MIME);
	}

	recentData->app_name = (gchar*) g_get_application_name();
	recentData->app_exec = g_strjoin(" ", g_get_prgname(), "%u", NULL);
	recentData->groups = groups;
	recentData->is_private = FALSE;

	GFile* file = g_file_new_for_path(filename.c_str());
	gchar* uri = g_file_get_uri(file);
	gtk_recent_manager_add_full(recentManager, uri, recentData);

	g_free(recentData->app_exec);

	g_slice_free(GtkRecentData, recentData);

	g_object_unref(file);
}

void RecentManager::removeRecentFileFilename(path filename)
{
	XOJ_CHECK_TYPE(RecentManager);

	GFile* file = g_file_new_for_path(filename.c_str());

	GtkRecentManager* recentManager = gtk_recent_manager_get_default();
	gtk_recent_manager_remove_item(recentManager, g_file_get_uri(file), NULL);

	g_object_unref(file);
}

int RecentManager::getMaxRecent()
{
	XOJ_CHECK_TYPE(RecentManager);

	return this->maxRecent;
}

void RecentManager::setMaxRecent(int maxRecent)
{
	XOJ_CHECK_TYPE(RecentManager);

	this->maxRecent = maxRecent;
}

void RecentManager::openRecent(path p)
{
	XOJ_CHECK_TYPE(RecentManager);

	if (p.filename().empty()) return;

	for (RecentManagerListener* l : this->listener)
	{
		l->fileOpened(p.c_str());
	}
}

GtkWidget* RecentManager::getMenu()
{
	XOJ_CHECK_TYPE(RecentManager);

	return menu;
}

void RecentManager::freeOldMenus()
{
	XOJ_CHECK_TYPE(RecentManager);

	for (GtkWidget* w : menuItemList)
	{
		gtk_widget_destroy(w);
	}

	this->menuItemList.clear();
}

int RecentManager::sortRecentsEntries(GtkRecentInfo* a, GtkRecentInfo* b)
{
	return (gtk_recent_info_get_modified(b) - gtk_recent_info_get_modified(a));
}

GList* RecentManager::filterRecent(GList* items, bool xoj)
{
	XOJ_CHECK_TYPE(RecentManager);

	GList* filteredItems = NULL;

	// filter
	for (GList* l = items; l != NULL; l = l->next)
	{
		GtkRecentInfo* info = (GtkRecentInfo*) l->data;
		string uri(gtk_recent_info_get_uri(info));

		// Skip remote files
		if (!ba::starts_with(uri, "file://")) continue;

		using namespace boost::filesystem;
		try {
			if (!exists(path(uri.substr(7)))) continue;	//substr is for removing uri's file://
		} catch (boost::filesystem::filesystem_error) {
			continue;
		}

		if (xoj && ba::ends_with(uri, ".xoj"))
		{
			filteredItems = g_list_prepend(filteredItems, info);
		}
		if (!xoj && ba::ends_with(uri, ".pdf"))
		{
			filteredItems = g_list_prepend(filteredItems, info);
		}
	}

	// sort
	filteredItems = g_list_sort(filteredItems, (GCompareFunc) sortRecentsEntries);

	return filteredItems;
}

void RecentManager::recentsMenuActivateCallback(GtkAction* action, RecentManager* recentManager)
{
	XOJ_CHECK_TYPE_OBJ(recentManager, RecentManager);

	GtkRecentInfo* info = (GtkRecentInfo*) g_object_get_data(G_OBJECT(action), "gtk-recent-info");
	g_return_if_fail(info != NULL);

	const gchar* uri = gtk_recent_info_get_uri(info); //topath
	recentManager->openRecent(string(uri).substr(7));
}

void RecentManager::addRecentMenu(GtkRecentInfo* info, int i)
{
	XOJ_CHECK_TYPE(RecentManager);

	string display_name(gtk_recent_info_get_display_name(info));
	ba::replace_all(display_name, "_", "__");	//escape underscore
	
	string label = (i >= 10
			? FS(bl::format("{1}. {2}") % i % display_name)
			: FS(bl::format("_{1}. {2}") % i % display_name));

	/* gtk_recent_info_get_uri_display (info) is buggy and
	 * works only for local files */
	GFile* gfile = g_file_new_for_uri(gtk_recent_info_get_uri(info));
	string ruri(g_file_get_parse_name(gfile));
	g_object_unref(gfile);
	ba::replace_first(ruri, g_get_home_dir(), "~/"); //replace home dir with tilde

	string tip = FS(C_F("{1} is a URI", "Open {1}") % ruri);

	
	GtkWidget* item = gtk_menu_item_new_with_mnemonic(label.c_str());

	gtk_widget_set_tooltip_text(item, tip.c_str());

	g_object_set_data_full(G_OBJECT(item), "gtk-recent-info", gtk_recent_info_ref(info),
						   (GDestroyNotify) gtk_recent_info_unref);

	g_signal_connect(item, "activate", G_CALLBACK(recentsMenuActivateCallback), this);

	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
	gtk_widget_set_visible(GTK_WIDGET(item), true);

	this->menuItemList.push_back(item);
}

void RecentManager::updateMenu()
{
	XOJ_CHECK_TYPE(RecentManager);

	GtkRecentManager* recentManager = gtk_recent_manager_get_default();
	GList* items = gtk_recent_manager_get_items(recentManager);
	GList* filteredItemsXoj = filterRecent(items, true);
	GList* filteredItemsPdf = filterRecent(items, false);

	freeOldMenus();

	int xojCount = 0;
	for (GList* l = filteredItemsXoj; l != NULL; l = l->next)
	{
		GtkRecentInfo* info = (GtkRecentInfo*) l->data;

		if (xojCount >= maxRecent)
		{
			break;
		}
		xojCount++;

		addRecentMenu(info, xojCount);
	}
	g_list_free(filteredItemsXoj);

	GtkWidget* separator = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), separator);
	gtk_widget_set_visible(GTK_WIDGET(separator), true);

	this->menuItemList.push_back(separator);

	int pdfCount = 0;
	for (GList* l = filteredItemsPdf; l != NULL; l = l->next)
	{
		GtkRecentInfo* info = (GtkRecentInfo*) l->data;

		if (pdfCount >= maxRecent)
		{
			break;
		}
		pdfCount++;

		addRecentMenu(info, pdfCount + xojCount);
	}
	g_list_free(filteredItemsPdf);

	g_list_foreach(items, (GFunc) gtk_recent_info_unref, NULL);
	g_list_free(items);
}
