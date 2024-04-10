#include "RecentManager.h"

#include <algorithm>   // for sort, max
#include <ctime>       // for time_t
#include <functional>  // for greater, less
#include <string>      // for string, allocator, operator+

#include "util/GListView.h"          // for GListView, GListView<>::GListVie...
#include "util/PathUtil.h"           // for fromUri, toUri, hasPdfFileExt
#include "util/TinyVector.h"         // for TinyVector
#include "util/safe_casts.h"         // for as_signed

#include "Control.h"

template <template <class> class Comp>
inline auto compareGtkRecentInfo(GtkRecentInfo const& a, GtkRecentInfo const& b) -> bool {
    auto deconstify = [](auto&& ref) { return const_cast<GtkRecentInfo*>(&ref); };  // NOLINT
    const GDateTime* a_time = gtk_recent_info_get_modified(deconstify(a));          // actually const
    const GDateTime* b_time = gtk_recent_info_get_modified(deconstify(b));
    xoj_assert(a_time && b_time);
    return Comp<int>{}(g_date_time_compare(a_time, b_time), 0);
}

inline auto operator<(GtkRecentInfo const& a, GtkRecentInfo const& b) -> bool {
    return compareGtkRecentInfo<std::less>(a, b);
}

inline auto operator>(GtkRecentInfo const& a, GtkRecentInfo const& b) -> bool {
    return compareGtkRecentInfo<std::greater>(a, b);
}

namespace {

enum AcceptedFileType { UNSUPPORTED_FILE_TYPE, XOURNAL_FILE_TYPE, PDF_FILE_TYPE };
auto getFileType(GtkRecentInfo& info) -> AcceptedFileType {
    const gchar* uri = gtk_recent_info_get_uri(&info);
    if (!uri) {  // issue #1071
        return UNSUPPORTED_FILE_TYPE;
    }
    auto p = Util::fromUri(uri);
    if (!p) {  // Skip remote files
        return UNSUPPORTED_FILE_TYPE;
    }
    if (Util::hasXournalFileExt(*p)) {
        return XOURNAL_FILE_TYPE;
    }
    if (Util::hasPdfFileExt(*p)) {
        return PDF_FILE_TYPE;
    }
    return UNSUPPORTED_FILE_TYPE;
}
}  // namespace

void RecentManager::clearRecentFiles() {
    GtkRecentManager* recentManager = gtk_recent_manager_get_default();
    GList* items = gtk_recent_manager_get_items(recentManager);
    auto item_view = GListView<GtkRecentInfo>(items);
    for (auto& recent: item_view) {
        if (getFileType(recent) != UNSUPPORTED_FILE_TYPE) {
            gtk_recent_manager_remove_item(recentManager,
                                           gtk_recent_info_get_uri(&recent),
                                           nullptr);
        }
    }
    g_list_free_full(items, GDestroyNotify(gtk_recent_info_unref));
}

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

auto RecentManager::getMostRecent() -> GtkRecentInfoSPtr {
    auto recent_items = gtk_recent_manager_get_items(gtk_recent_manager_get_default());
    // Todo (cpp20): replace with std::ranges::max_element
    if (!recent_items) {
        return nullptr;
    }
    GtkRecentInfo* mostRecent = static_cast<GtkRecentInfo*>(recent_items->data);
    for (auto& recent: GListView<GtkRecentInfo>(recent_items->next)) {
        if (getFileType(recent) != XOURNAL_FILE_TYPE) {
            continue;
        }
        if (*mostRecent < recent) {
            mostRecent = &recent;
        }
    }
    // Prolong lifetime past the list deletion
    GtkRecentInfoSPtr res(mostRecent, xoj::util::ref);

    g_list_free_full(recent_items, GDestroyNotify(gtk_recent_info_unref));

    return res;
}

auto RecentManager::getRecentFiles() -> RecentFiles {
    GtkRecentManager* recentManager = gtk_recent_manager_get_default();
    GList* items = gtk_recent_manager_get_items(recentManager);

    RecentFiles res;
    for (auto& recent: GListView<GtkRecentInfo>(items)) {
        auto fileType = getFileType(recent);
        if (fileType == PDF_FILE_TYPE && res.recentPdfFiles.size() < MAX_RECENT) {
            res.recentPdfFiles.emplace_back(&recent, xoj::util::ref);
        }
        if (fileType == XOURNAL_FILE_TYPE && res.recentXoppFiles.size() < MAX_RECENT) {
            res.recentXoppFiles.emplace_back(&recent, xoj::util::ref);
        }
    }
    g_list_free_full(items, GDestroyNotify(gtk_recent_info_unref));

    auto comp = [](const GtkRecentInfoSPtr& p1, const GtkRecentInfoSPtr& p2) { return *p1.get() > *p2.get(); };

    std::sort(res.recentPdfFiles.begin(), res.recentPdfFiles.end(), comp);
    std::sort(res.recentXoppFiles.begin(), res.recentXoppFiles.end(), comp);

    return res;
}
