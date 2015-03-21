#include <gtk/gtk.h>
#include <boost/filesystem.hpp>
#include <iostream>

#include "RecentManager.h"

#include <config.h>

using namespace std;

#define MIME "application/x-xoj"
#define MIME_PDF "application/x-pdf"
#define GROUP "xournal++"

RecentManager::RecentManager() {
    XOJ_INIT_TYPE(RecentManager);

    this->maxRecent = 10;
    this->menu = gtk_menu_new();
    this->menuItemList = NULL;
    this->listener = NULL;

    GtkRecentManager* recentManager = gtk_recent_manager_get_default();
    this->recentHandlerId = g_signal_connect(recentManager, "changed",
            G_CALLBACK(recentManagerChangedCallback), this);

    gdk_threads_enter();
    updateMenu();
    gdk_threads_leave();
}

RecentManager::~RecentManager() {
    XOJ_CHECK_TYPE(RecentManager);

    if (this->recentHandlerId) {
        GtkRecentManager* recentManager = gtk_recent_manager_get_default();
        g_signal_handler_disconnect(recentManager, this->recentHandlerId);
        this->recentHandlerId = 0;
    }
    this->menu = NULL;

    g_list_free(this->listener);
    this->listener = NULL;

    XOJ_RELEASE_TYPE(RecentManager);
}

void RecentManager::addListener(RecentManagerListener* listener) {
    XOJ_CHECK_TYPE(RecentManager);

    this->listener = g_list_append(this->listener, listener);
}

void RecentManager::recentManagerChangedCallback(GtkRecentManager* manager,
        RecentManager* recentManager) {
    // regenerate the menu when the model changes
    recentManager->updateMenu();
}

void RecentManager::addRecentFileFilename(string filename) {
    XOJ_CHECK_TYPE(RecentManager);

    cout << bl::format("addRecentFileFilename: {1}") % filename << endl;

    const gchar* c_filename = filename.c_str();
    if (filename.compare(0, 7, "file://") == 0) {
        addRecentFileUri(c_filename);
        return;
    }

    GFile* file = g_file_new_for_path(c_filename);

    addRecentFileUri(g_file_get_uri(file));

    g_object_unref(file);
}

void RecentManager::addRecentFileUri(string uri) {
    XOJ_CHECK_TYPE(RecentManager);

    cout << bl::format("addRecentFileUri: {1}") % uri << endl;

    GtkRecentManager* recentManager;
    GtkRecentData* recentData;

    static gchar * groups[2] = {g_strdup(GROUP), NULL};

    recentManager = gtk_recent_manager_get_default();

    recentData = g_slice_new(GtkRecentData);

    recentData->display_name = NULL;
    recentData->description = NULL;

    if (ba::ends_with(uri, ".pdf")) {
        recentData->mime_type = (gchar*) g_strdup(MIME_PDF);
    } else {
        recentData->mime_type = (gchar*) g_strdup(MIME);
    }

    recentData->app_name = (gchar*) g_get_application_name();
    recentData->app_exec = g_strjoin(" ", g_get_prgname(), "%u", NULL);
    recentData->groups = groups;
    recentData->is_private = FALSE;

    gtk_recent_manager_add_full(recentManager, uri.c_str(), recentData);

    g_free(recentData->app_exec);

    g_slice_free(GtkRecentData, recentData);
}

void RecentManager::removeRecentFileFilename(string filename) {
    XOJ_CHECK_TYPE(RecentManager);

    GFile* file = g_file_new_for_path(filename.c_str());

    removeRecentFileUri(g_file_get_uri(file));

    g_object_unref(file);
}

void RecentManager::removeRecentFileUri(string uri) {
    XOJ_CHECK_TYPE(RecentManager);

    GtkRecentManager* recentManager = gtk_recent_manager_get_default();
    gtk_recent_manager_remove_item(recentManager, uri.c_str(), NULL);
}

int RecentManager::getMaxRecent() {
    XOJ_CHECK_TYPE(RecentManager);

    return this->maxRecent;
}

void RecentManager::setMaxRecent(int maxRecent) {
    XOJ_CHECK_TYPE(RecentManager);

    this->maxRecent = maxRecent;
}

void RecentManager::openRecent(string uri) {
    XOJ_CHECK_TYPE(RecentManager);

    const gchar* c_uri = uri.c_str();
    if (!ba::starts_with(uri, "file://")) {
        g_warning("could not handle URI: %s", c_uri);
        return;
    }

    string filename = uri.substr(uri.find_last_of('/'));
    if (filename.empty()) {
        return;
    }

    for (GList* l = this->listener; l != NULL; l = l->next) {
        RecentManagerListener* listener = (RecentManagerListener*) l->data;
        listener->fileOpened(filename.c_str());
    }
}

GtkWidget* RecentManager::getMenu() {
    XOJ_CHECK_TYPE(RecentManager);

    return menu;
}

void RecentManager::freeOldMenus() {
    XOJ_CHECK_TYPE(RecentManager);

    GtkWidget* w = NULL;

    for (GList* l = menuItemList; l != NULL; l = l->next) {
        w = (GtkWidget*) l->data;

        gtk_widget_destroy(w);
    }

    g_list_free(this->menuItemList);
    this->menuItemList = NULL;
}

/*
 * Doubles underscore to avoid spurious menu accels.
 */
gchar* gedit_utils_escape_underscores(const gchar* text, gssize length) {
    string r(text, length);
    ba::replace_all(r, "_", "__");
    return const_cast<gchar*>(r.c_str());
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
gchar* gedit_utils_uri_for_display(const gchar* uri) {
    GFile* gfile;
    gchar* parse_name;

    gfile = g_file_new_for_uri(uri);
    parse_name = g_file_get_parse_name(gfile);
    g_object_unref(gfile);

    return parse_name;
}

gchar* gedit_utils_replace_home_dir_with_tilde(const gchar* uri) {
    string r(uri);
    ba::replace_all(r, g_get_home_dir(), "~/");
    return const_cast<gchar*>(r.c_str());
}

int RecentManager::sortRecentsEntries(GtkRecentInfo* a, GtkRecentInfo* b) {
    return (gtk_recent_info_get_modified(b) - gtk_recent_info_get_modified(a));
}

GList* RecentManager::filterRecent(GList* items, bool xoj) {
    XOJ_CHECK_TYPE(RecentManager);

    GList* filteredItems = NULL;

    // filter
    for (GList* l = items; l != NULL; l = l->next) {
        GtkRecentInfo* info = (GtkRecentInfo*) l->data;
        string uri(gtk_recent_info_get_uri(info));

        // Skip remote files anyway, PDF are supported as remote, XOJ not
        if (!ba::starts_with(uri, "file://")) continue;
        
        using namespace boost::filesystem;
        if (!exists(path(uri))) continue;

        if (xoj) {
            if (ba::ends_with(uri, ".xoj")) {
                filteredItems = g_list_prepend(filteredItems, info);
            }
        } else {
            if (ba::ends_with(uri, ".pdf")) {
                filteredItems = g_list_prepend(filteredItems, info);
            }
        }
    }

    // sort
    filteredItems = g_list_sort(filteredItems, (GCompareFunc) sortRecentsEntries);

    return filteredItems;
}

void RecentManager::recentsMenuActivateCallback(GtkAction* action,
        RecentManager* recentManager) {
    XOJ_CHECK_TYPE_OBJ(recentManager, RecentManager);

    GtkRecentInfo* info = (GtkRecentInfo*) g_object_get_data(G_OBJECT(action),
            "gtk-recent-info");
    g_return_if_fail(info != NULL);

    const gchar* uri = gtk_recent_info_get_uri(info);
    recentManager->openRecent(uri);
}

void RecentManager::addRecentMenu(GtkRecentInfo* info, int i) {
    XOJ_CHECK_TYPE(RecentManager);

    gchar* label = NULL;
    const gchar* display_name = gtk_recent_info_get_display_name(info);
    gchar* escaped = gedit_utils_escape_underscores(display_name, -1);
    if (i >= 10) {
        label = g_strdup_printf("%d.  %s", i, escaped);
    } else {
        label = g_strdup_printf("_%d.  %s", i, escaped);
    }
    g_free(escaped);

    /* gtk_recent_info_get_uri_display (info) is buggy and
     * works only for local files */
    gchar* uri = gedit_utils_uri_for_display(gtk_recent_info_get_uri(info));

    gchar* ruri = gedit_utils_replace_home_dir_with_tilde(uri);
    g_free(uri);

    // Translators: %s is a URI
    gchar* tip = g_strdup_printf("Open '%s'", ruri);
    g_free(ruri);

    GtkWidget* item = gtk_menu_item_new_with_mnemonic(label);

    gtk_widget_set_tooltip_text(item, tip);

    g_object_set_data_full(G_OBJECT(item), "gtk-recent-info",
            gtk_recent_info_ref(info), (GDestroyNotify) gtk_recent_info_unref);

    g_signal_connect(item, "activate", G_CALLBACK(recentsMenuActivateCallback), this);

    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
    gtk_widget_set_visible(GTK_WIDGET(item), true);

    this->menuItemList = g_list_append(this->menuItemList, item);

    g_free(label);
    g_free(tip);
}

void RecentManager::updateMenu() {
    XOJ_CHECK_TYPE(RecentManager);

    GtkRecentManager* recentManager = gtk_recent_manager_get_default();
    GList* items = gtk_recent_manager_get_items(recentManager);
    GList* filteredItemsXoj = filterRecent(items, true);
    GList* filteredItemsPdf = filterRecent(items, false);

    freeOldMenus();

    int i = 0;
    for (GList* l = filteredItemsXoj; l != NULL; l = l->next) {
        GtkRecentInfo* info = (GtkRecentInfo*) l->data;

        if (i >= maxRecent) {
            break;
        }
        i++;

        addRecentMenu(info, i);
    }

    GtkWidget* separator = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), separator);
    gtk_widget_set_visible(GTK_WIDGET(separator), true);

    this->menuItemList = g_list_append(this->menuItemList, separator);

    i = 0;
    for (GList* l = filteredItemsPdf; l != NULL; l = l->next) {
        GtkRecentInfo* info = (GtkRecentInfo*) l->data;

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

