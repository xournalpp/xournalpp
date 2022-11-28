#include "RecentManager.h"

#include <algorithm>    // for sort, max
#include <array>        // for array
#include <ctime>        // for time_t
#include <functional>   // for reference_wrapper, greater, less
#include <optional>     // for optional
#include <string>       // for string, allocator, operator+
#include <type_traits>  // for make_signed
#include <vector>       // for vector

#include <gio/gio.h>      // for g_file_get_parse_name, g_file_ne...
#include <glib-object.h>  // for g_object_get_data, g_object_set_...

#include "util/GListView.h"          // for GListView, GListView<>::GListVie...
#include "util/PathUtil.h"           // for fromUri, toUri, hasPdfFileExt
#include "util/PlaceholderString.h"  // for PlaceholderString
#include "util/SmallVector.h"        // for SmallVector
#include "util/StringUtils.h"        // for replace_pair, StringUtils
#include "util/i18n.h"               // for FS, FORMAT_STR, C_F
#include "util/safe_casts.h"         // for as_signed


constexpr auto const* MIME = "application/x-xoj";
constexpr auto const* MIME_PDF = "application/x-pdf";
constexpr auto const* GROUP = "xournal++";
constexpr int maxRecent = 10;

template <template <class> class Comp>
inline auto compareGtkRecentInfo(GtkRecentInfo const& a, GtkRecentInfo const& b) -> bool {
    auto deconstify = [](auto&& ref) { return const_cast<GtkRecentInfo*>(&ref); };  // NOLINT
    auto a_time = gtk_recent_info_get_modified(deconstify(a));                      // actually const
    auto b_time = gtk_recent_info_get_modified(deconstify(b));
    return Comp<time_t>{}(a_time, b_time);
}

inline auto operator<(GtkRecentInfo const& a, GtkRecentInfo const& b) -> bool {
    return compareGtkRecentInfo<std::less>(a, b);
}

inline auto operator>(GtkRecentInfo const& a, GtkRecentInfo const& b) -> bool {
    return compareGtkRecentInfo<std::greater>(a, b);
}

namespace {

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

void recentsClearActivateCallback(GtkMenuItem* self, GtkRecentManager* recentManager) {
    GList* items = gtk_recent_manager_get_items(recentManager);
    auto item_view = GListView<GtkRecentInfo>(items);
    for (auto& recent: item_view) {
        if (filterRecent(recent, false) ||
            filterRecent(recent, true)) {
            gtk_recent_manager_remove_item(recentManager,
                                           gtk_recent_info_get_uri(&recent),
                                           nullptr);
        }
    }
    g_list_free_full(items, GDestroyNotify(gtk_recent_info_unref));
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
    auto item_view = GListView<GtkRecentInfo>(items);

    SmallVector<std::reference_wrapper<GtkRecentInfo>, maxRecent> xoj_files;
    SmallVector<std::reference_wrapper<GtkRecentInfo>, maxRecent> pdf_files;

    for (auto& recent: item_view) {
        if (filterRecent(recent, false) && pdf_files.size() < maxRecent) {
            pdf_files.emplace_back(recent);
        }
        if (filterRecent(recent, true) && xoj_files.size() < maxRecent) {
            xoj_files.emplace_back(recent);
        }
    }

    if (pdf_files.empty() && xoj_files.empty()) {
        GtkWidget* noFilesItem = gtk_menu_item_new_with_mnemonic(
            _("No recent files"));
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), noFilesItem);
        gtk_widget_set_visible(GTK_WIDGET(noFilesItem), true);
        gtk_widget_set_sensitive(GTK_WIDGET(noFilesItem), false);
        this->menuItemList.push_back(noFilesItem);
        g_list_free_full(items, GDestroyNotify(gtk_recent_info_unref));
        return;
    }

    std::sort(xoj_files.begin(), xoj_files.end(), std::greater<GtkRecentInfo>());
    std::sort(pdf_files.begin(), pdf_files.end(), std::greater<GtkRecentInfo>());

    auto insert_items_job = [this](auto&& container, auto const base) mutable {
        auto const& begi = container.begin();
        auto const& endi = container.end();
        auto item_count = base;
        for (auto iter = begi; iter != endi; ++iter, ++item_count) {
            addRecentMenu(&iter->get(), item_count);
        }
        return item_count;
    };

    auto base = insert_items_job(xoj_files, 0);

    if (!xoj_files.empty()) {
        GtkWidget* separatorTop = gtk_separator_menu_item_new();
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), separatorTop);
        gtk_widget_set_visible(GTK_WIDGET(separatorTop), true);
        this->menuItemList.push_back(separatorTop);
    }
    insert_items_job(pdf_files, base);

    if (!pdf_files.empty()) {
        GtkWidget* separatorBottom = gtk_separator_menu_item_new();
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), separatorBottom);
        gtk_widget_set_visible(GTK_WIDGET(separatorBottom), true);
        this->menuItemList.push_back(separatorBottom);
    }

    GtkWidget* clearItem = gtk_menu_item_new_with_mnemonic(
        _("Clear list"));
    g_signal_connect(clearItem, "activate",
                    G_CALLBACK(recentsClearActivateCallback), recentManager);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), clearItem);
    gtk_widget_set_visible(GTK_WIDGET(clearItem), true);
    this->menuItemList.push_back(clearItem);

    g_list_free_full(items, GDestroyNotify(gtk_recent_info_unref));
}
