#include "RecentManager.h"

#include <array>

#include <filesystem.h>

#include "util/GListView.h"
#include "util/PathUtil.h"
#include "util/StringUtils.h"
#include "util/i18n.h"
#include "util/safe_casts.h"

constexpr auto const* MIME = "application/x-xoj";
constexpr auto const* MIME_PDF = "application/x-pdf";
constexpr auto const* GROUP = "xournal++";

namespace {

auto operator<(GtkRecentInfo& a, GtkRecentInfo& b) -> bool {
    auto a_time = gtk_recent_info_get_modified(&a);
    auto b_time = gtk_recent_info_get_modified(&b);
    return a_time < b_time;
}

auto filterRecent(GtkRecentInfo& info, bool xoj) -> bool {
    const gchar* uri = gtk_recent_info_get_uri(&info);
    if (!uri) {  // issue #1071
        return false;
    }
    auto p = Util::fromUri(uri);
    if (!p) {  // Skip remote files
        return false;
    }
    return (xoj && Util::hasXournalFileExt(*p)) || (!xoj && p->extension() == ".pdf");
}

void recentManagerChangedCallback(GtkRecentManager* /*manager*/, RecentManager* recentManager) {
    recentManager->updateMenu();
}

void recentsMenuActivateCallback(GtkMenuItem* self, RecentManager* recentManager) {
    auto* info = static_cast<GtkRecentInfo*>(g_object_get_data(G_OBJECT(self), "gtk-recent-info"));
    g_return_if_fail(info != nullptr);

    auto p = Util::fromUri(gtk_recent_info_get_uri(info));
    if (p) {
        recentManager->openRecent(*p);
    }
}

}  // namespace

RecentManagerListener::~RecentManagerListener() = default;

RecentManager::RecentManager() {
    this->menu = gtk_menu_new();

    GtkRecentManager* recentManager = gtk_recent_manager_get_default();
    this->recentHandlerId = g_signal_connect(recentManager, "changed", G_CALLBACK(recentManagerChangedCallback), this);

    updateMenu();
}

RecentManager::~RecentManager() {
    if (this->recentHandlerId) {
        GtkRecentManager* recentManager = gtk_recent_manager_get_default();
        g_signal_handler_disconnect(recentManager, this->recentHandlerId);
        this->recentHandlerId = 0;
    }
    this->menu = nullptr;
}

void RecentManager::addListener(RecentManagerListener* l) { this->listener.push_back(l); }

void RecentManager::addRecentFileFilename(const fs::path& filepath) {
    GtkRecentManager* recentManager = gtk_recent_manager_get_default();

    std::string group_name = GROUP;
    std::array<gchar*, 2> groups = {group_name.data(), nullptr};
    std::string app_name = g_get_application_name();
    std::string app_exec = std::string(g_get_prgname()) + " %u";
    std::string mime_type = Util::hasPdfFileExt(filepath) ? std::string(MIME_PDF) : std::string(MIME);

    GtkRecentData recentData{};
    recentData.display_name = nullptr;
    recentData.description = nullptr;
    recentData.app_name = app_name.data();
    recentData.app_exec = app_exec.data();
    recentData.groups = groups.data();
    recentData.mime_type = mime_type.data();
    recentData.is_private = false;

    auto uri = Util::toUri(filepath);
    if (!uri) {
        return;
    }
    gtk_recent_manager_add_full(recentManager, (*uri).c_str(), &recentData);
}

void RecentManager::removeRecentFileFilename(const fs::path& filename) {
    auto uri = Util::toUri(filename);
    if (!uri) {
        return;
    }
    GtkRecentManager* recentManager = gtk_recent_manager_get_default();
    gtk_recent_manager_remove_item(recentManager, uri->c_str(), nullptr);
}

void RecentManager::openRecent(const fs::path& p) {
    if (p.empty()) {
        return;
    }

    for (RecentManagerListener* l: this->listener) { l->fileOpened(p); }
}

auto RecentManager::getMenu() -> GtkWidget* { return menu; }

void RecentManager::freeOldMenus() {
    for (GtkWidget* w: menuItemList) { gtk_widget_destroy(w); }

    this->menuItemList.clear();
}

using stime_t = std::make_signed<time_t>;

auto RecentManager::getMostRecent() -> GtkRecentInfo* {
    auto recent_items = gtk_recent_manager_get_items(gtk_recent_manager_get_default());
    // Todo (cpp20): replace with std::ranges::max_element
    if (!recent_items) {
        return nullptr;
    }
    GtkRecentInfo* mostRecent = static_cast<GtkRecentInfo*>(recent_items->data);
    for (auto& recent: GListView<GtkRecentInfo>(recent_items->next)) {
        auto time = gtk_recent_info_get_modified(&recent);
        if (as_signed(time) < 0) {
            continue;
        }
        if (!filterRecent(recent, true)) {
            continue;
        }
        if (*mostRecent < recent) {
            mostRecent = &recent;
        }
    }
    gtk_recent_info_ref(mostRecent);

    for (auto& recent_info: GListView<GtkRecentInfo>(recent_items)) { gtk_recent_info_unref(&recent_info); }
    g_list_free(recent_items);
    return mostRecent;
}

void RecentManager::addRecentMenu(GtkRecentInfo* info, int i) {
    std::string display_name = gtk_recent_info_get_display_name(info);

    // escape underscore
    StringUtils::replaceAllChars(display_name, {replace_pair('_', "__")});

    std::string label =
            (i >= 10 ? FS(FORMAT_STR("{1}. {2}") % i % display_name) : FS(FORMAT_STR("_{1}. {2}") % i % display_name));

    /* gtk_recent_info_get_uri_display (info) is buggy and
     * works only for local files */
    GFile* gfile = g_file_new_for_uri(gtk_recent_info_get_uri(info));
    char* fileUri = g_file_get_parse_name(gfile);
    std::string ruri = fileUri;
    g_free(fileUri);

    g_object_unref(gfile);

    if (StringUtils::startsWith(ruri, "~/")) {
        ruri = std::string(g_get_home_dir()) + ruri.substr(1);
    }

    std::string tip = FS(C_F("{1} is a URI", "Open {1}") % ruri);


    GtkWidget* item = gtk_menu_item_new_with_mnemonic(label.c_str());

    gtk_widget_set_tooltip_text(item, tip.c_str());

    g_object_set_data_full(G_OBJECT(item), "gtk-recent-info", gtk_recent_info_ref(info),
                           reinterpret_cast<GDestroyNotify>(gtk_recent_info_unref));

    g_signal_connect(item, "activate", G_CALLBACK(recentsMenuActivateCallback), this);

    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
    gtk_widget_set_visible(GTK_WIDGET(item), true);

    this->menuItemList.push_back(item);
}

void RecentManager::updateMenu() {
    freeOldMenus();

    GtkRecentManager* recentManager = gtk_recent_manager_get_default();
    GList* items = gtk_recent_manager_get_items(recentManager);

    auto insert_items_job = [=](auto&& filter_fn, auto base) mutable {
        auto item_count = base;
        auto max = maxRecent + base;
        for (auto& info: GListView<GtkRecentInfo>(items)) {
            if (!filter_fn(info)) {
                continue;
            }
            if (item_count >= max) {
                break;
            }
            ++item_count;
            addRecentMenu(&info, item_count);
        }
        return item_count;
    };

    auto base = insert_items_job([](auto&& item) { return filterRecent(item, true); }, 0);

    GtkWidget* separator = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), separator);
    gtk_widget_set_visible(GTK_WIDGET(separator), true);
    this->menuItemList.push_back(separator);

    insert_items_job([](auto&& item) { return filterRecent(item, false); }, base);

    g_list_foreach(items, reinterpret_cast<GFunc>(gtk_recent_info_unref), nullptr);
    g_list_free(items);
}
