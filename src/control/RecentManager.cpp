#include <gtk/gtk.h>
#include <string.h>
#include "RecentManager.h"

#include <config.h>
#include <glib/gi18n-lib.h>

#define MIME "application/x-xoj"
#define MIME_PDF "application/x-pdf"
#define GROUP "xournal++"

RecentManager::RecentManager() {
	XOJ_INIT_TYPE(RecentManager);

	this->maxRecent = 10;
	this->menu = gtk_menu_new();
	this->menuItemList = NULL;
	this->listener = NULL;

	GtkRecentManager * recentManager = gtk_recent_manager_get_default();
	this->recentHandlerId = g_signal_connect(recentManager, "changed", G_CALLBACK (recentManagerChangedCallback), this);

	updateMenu();
}

RecentManager::~RecentManager() {
	XOJ_CHECK_TYPE(RecentManager);

	if (this->recentHandlerId) {
		GtkRecentManager * recentManager = gtk_recent_manager_get_default();
		g_signal_handler_disconnect(recentManager, this->recentHandlerId);
		this->recentHandlerId = 0;
	}
	this->menu = NULL;

	g_list_free(this->listener);
	this->listener = NULL;

	XOJ_RELEASE_TYPE(RecentManager);
}

void RecentManager::addListener(RecentManagerListener * listener) {
	XOJ_CHECK_TYPE(RecentManager);

	this->listener = g_list_append(this->listener, listener);
}

void RecentManager::recentManagerChangedCallback(GtkRecentManager * manager, RecentManager * recentManager) {
	// regenerate the menu when the model changes
	recentManager->updateMenu();
}

void RecentManager::addRecentFileFilename(const char * filename) {
	XOJ_CHECK_TYPE(RecentManager);

	printf("addRecentFileFilename: %s\n", filename);

	if (strncmp(filename, "file://", 7) == 0) {
		addRecentFileUri(filename);
		return;
	}

	GFile * file = g_file_new_for_path(filename);

	addRecentFileUri(g_file_get_uri(file));

	g_object_unref(file);
}

void RecentManager::addRecentFileUri(const char * uri) {
	XOJ_CHECK_TYPE(RecentManager);

	printf("addRecentFileUri: %s\n", uri);

	GtkRecentManager * recentManager;
	GtkRecentData * recentData;

	static gchar * groups[2] = { g_strdup(GROUP), NULL };

	recentManager = gtk_recent_manager_get_default();

	recentData = g_slice_new(GtkRecentData);

	recentData->display_name = NULL;
	recentData->description = NULL;

	if (g_str_has_suffix(uri, ".pdf")) {
		recentData->mime_type = (gchar *) g_strdup(MIME_PDF);
	} else {
		recentData->mime_type = (gchar *) g_strdup(MIME);
	}

	recentData->app_name = (gchar *) g_get_application_name();
	recentData->app_exec = g_strjoin(" ", g_get_prgname(), "%u", NULL);
	recentData->groups = groups;
	recentData->is_private = FALSE;

	gtk_recent_manager_add_full(recentManager, uri, recentData);

	g_free(recentData->app_exec);

	g_slice_free(GtkRecentData, recentData);
}

void RecentManager::removeRecentFileFilename(const char * filename) {
	XOJ_CHECK_TYPE(RecentManager);

	GFile * file = g_file_new_for_path(filename);

	removeRecentFileUri(g_file_get_uri(file));

	g_object_unref(file);
}

void RecentManager::removeRecentFileUri(const char * uri) {
	XOJ_CHECK_TYPE(RecentManager);

	GtkRecentManager * recentManager = gtk_recent_manager_get_default();
	gtk_recent_manager_remove_item(recentManager, uri, NULL);
}

int RecentManager::getMaxRecent() {
	XOJ_CHECK_TYPE(RecentManager);

	return this->maxRecent;
}

void RecentManager::setMaxRecent(int maxRecent) {
	XOJ_CHECK_TYPE(RecentManager);

	this->maxRecent = maxRecent;
}

void RecentManager::openRecent(String uri) {
	XOJ_CHECK_TYPE(RecentManager);

	if (!uri.startsWith("file://")) {
		g_warning("could not handle URI: %s", uri.c_str());
		return;
	}

	char * filename = g_filename_from_uri(uri.c_str(), NULL, NULL);
	if (!filename) {
		return;
	}

	for (GList * l = this->listener; l != NULL; l = l->next) {
		RecentManagerListener * listener = (RecentManagerListener *) l->data;
		listener->fileOpened(filename);
	}
}

GtkWidget * RecentManager::getMenu() {
	XOJ_CHECK_TYPE(RecentManager);

	return menu;
}

void RecentManager::freeOldMenus() {
	XOJ_CHECK_TYPE(RecentManager);

	GtkWidget * w = NULL;

	for (GList * l = menuItemList; l != NULL; l = l->next) {
		w = (GtkWidget *) l->data;
		gtk_widget_destroy(w);
	}

	g_list_free(this->menuItemList);
	this->menuItemList = NULL;
}

/*
 * Doubles underscore to avoid spurious menu accels.
 */
gchar *
gedit_utils_escape_underscores(const gchar* text, gssize length) {
	GString *str;
	const gchar *p;
	const gchar *end;

	g_return_val_if_fail (text != NULL, NULL);

	if (length < 0) {
		length = strlen(text);
	}

	str = g_string_sized_new(length);

	p = text;
	end = text + length;

	while (p != end) {
		const gchar *next;
		next = g_utf8_next_char (p);

		switch (*p) {
		case '_':
			g_string_append(str, "__");
			break;
		default:
			g_string_append_len(str, p, next - p);
			break;
		}

		p = next;
	}

	return g_string_free(str, FALSE);
}

/**
 * gedit_utils_uri_for_display:
 * @uri: uri to be displayed.
 *
 * Filter, modify, unescape and change @uri to make it appropriate
 * for display to users.
 *
 * This function is a convenient wrapper for g_file_get_parse_name
 *
 * Return value: a string which represents @uri and can be displayed.
 */
gchar *
gedit_utils_uri_for_display(const gchar *uri) {
	GFile *gfile;
	gchar *parse_name;

	gfile = g_file_new_for_uri(uri);
	parse_name = g_file_get_parse_name(gfile);
	g_object_unref(gfile);

	return parse_name;
}

gchar *
gedit_utils_replace_home_dir_with_tilde(const gchar *uri) {
	gchar *tmp;
	gchar *home;

	g_return_val_if_fail (uri != NULL, NULL);

	/* Note that g_get_home_dir returns a const string */
	tmp = (gchar *) g_get_home_dir();

	if (tmp == NULL)
		return g_strdup(uri);

	home = g_filename_to_utf8(tmp, -1, NULL, NULL, NULL);
	if (home == NULL)
		return g_strdup(uri);

	if (strcmp(uri, home) == 0) {
		g_free(home);

		return g_strdup("~");
	}

	tmp = home;
	home = g_strdup_printf("%s/", tmp);
	g_free(tmp);

	if (g_str_has_prefix(uri, home)) {
		gchar *res;

		res = g_strdup_printf("~/%s", uri + strlen(home));

		g_free(home);

		return res;
	}

	g_free(home);

	return g_strdup(uri);
}

int RecentManager::sortRecentsEntries(GtkRecentInfo * a, GtkRecentInfo * b) {
	return (gtk_recent_info_get_modified(b) - gtk_recent_info_get_modified(a));
}

GList * RecentManager::filterRecent(GList * items, bool xoj) {
	XOJ_CHECK_TYPE(RecentManager);

	GList * filteredItems = NULL;

	// filter
	for (GList * l = items; l != NULL; l = l->next) {
		GtkRecentInfo * info = (GtkRecentInfo *) l->data;
		const char * uri = gtk_recent_info_get_uri(info);

		// Skip remote files anyway, PDF are supported as remote, XOJ not
		if (!g_str_has_prefix(uri, "file://")) {
			continue;
		}

		GFile * file = g_file_new_for_uri(uri);

		// skipt not existing files
		if (!g_file_query_exists(file, NULL)) {
			g_object_unref(file);
			continue;
		}
		g_object_unref(file);

		if (xoj) {
			if (g_str_has_suffix(uri, ".xoj")) {
				filteredItems = g_list_prepend(filteredItems, info);
			}
		} else {
			if (g_str_has_suffix(uri, ".pdf")) {
				filteredItems = g_list_prepend(filteredItems, info);
			}
		}
	}

	// sort
	filteredItems = g_list_sort(filteredItems, (GCompareFunc) sortRecentsEntries);

	return filteredItems;
}

void RecentManager::recentsMenuActivateCallback(GtkAction * action, RecentManager * recentManager) {
	XOJ_CHECK_TYPE_OBJ(recentManager, RecentManager);

	GtkRecentInfo * info = (GtkRecentInfo *) g_object_get_data(G_OBJECT (action), "gtk-recent-info");
	g_return_if_fail(info != NULL);

	const char * uri = gtk_recent_info_get_uri(info);
	recentManager->openRecent(uri);
	gtk_recent_info_unref(info);
}

void RecentManager::addRecentMenu(GtkRecentInfo * info, int i) {
	XOJ_CHECK_TYPE(RecentManager);

	gchar * label = NULL;
	const char * display_name = gtk_recent_info_get_display_name(info);
	char * escaped = gedit_utils_escape_underscores(display_name, -1);
	if (i >= 10) {
		label = g_strdup_printf("%d.  %s", i, escaped);
	} else {
		label = g_strdup_printf("_%d.  %s", i, escaped);
	}
	g_free(escaped);

	/* gtk_recent_info_get_uri_display (info) is buggy and
	 * works only for local files */
	char * uri = gedit_utils_uri_for_display(gtk_recent_info_get_uri(info));

	char * ruri = gedit_utils_replace_home_dir_with_tilde(uri);
	g_free(uri);

	// Translators: %s is a URI
	char * tip = g_strdup_printf(_("Open '%s'"), ruri);
	g_free(ruri);

	GtkWidget * item = gtk_menu_item_new_with_mnemonic(label);

	gtk_widget_set_tooltip_text(item, tip);

	g_object_set_data_full(G_OBJECT (item), "gtk-recent-info", gtk_recent_info_ref(info), (GDestroyNotify) gtk_recent_info_unref);

	g_signal_connect (item,
			"activate",
			G_CALLBACK (recentsMenuActivateCallback),
			this);

	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
	gtk_widget_set_visible(GTK_WIDGET(item), true);

	this->menuItemList = g_list_append(this->menuItemList, item);

	g_free(label);
	g_free(tip);
}

void RecentManager::updateMenu() {
	XOJ_CHECK_TYPE(RecentManager);

	GtkRecentManager * recentManager = gtk_recent_manager_get_default();
	GList * items = gtk_recent_manager_get_items(recentManager);
	GList * filteredItemsXoj = filterRecent(items, true);
	GList * filteredItemsPdf = filterRecent(items, false);

	freeOldMenus();

	int i = 0;
	for (GList * l = filteredItemsXoj; l != NULL; l = l->next) {
		GtkRecentInfo * info = (GtkRecentInfo *) l->data;

		if (i >= maxRecent) {
			break;
		}
		i++;

		addRecentMenu(info, i);
	}

	GtkWidget * separator = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), separator);
	gtk_widget_set_visible(GTK_WIDGET(separator), true);
	this->menuItemList = g_list_append(this->menuItemList, separator);

	i = 0;
	for (GList * l = filteredItemsPdf; l != NULL; l = l->next) {
		GtkRecentInfo * info = (GtkRecentInfo *) l->data;

		if (i >= maxRecent) {
			break;
		}
		i++;

		addRecentMenu(info, i + maxRecent);
	}

	g_list_free(filteredItemsXoj);
	g_list_free(filteredItemsPdf);

	g_list_foreach(items, (GFunc) gtk_recent_info_unref, NULL);
	g_list_free(items);
}

