#include "RecentDocumentsSubmenu.h"

#include <string>  // for string, allocator, operator+

#include <gio/gio.h>  // for GMenu

#include "control/Control.h"
#include "control/RecentManager.h"
#include "gui/menus/StaticAssertActionNamespace.h"
#include "util/Assert.h"
#include "util/PathUtil.h"           // for fromUri, toUri, hasPdfFileExt
#include "util/PlaceholderString.h"  // for PlaceholderString
#include "util/StringUtils.h"        // for replace_pair, StringUtils
#include "util/TinyVector.h"         // for TinyVector
#include "util/i18n.h"               // for FS, FORMAT_STR, C_F

#include "Menubar.h"

namespace {
constexpr auto SUBMENU_ID = "menuFileRecent";
constexpr auto G_ACTION_NAMESPACE = "win.";
constexpr auto OPEN_ACTION_NAME = "open-file-at";

/**
 * @brief For a disable placeholder saying "No recent files"
 *  To disable a GMenu entry, give it an action name that does not correspond to any GAction
 */
constexpr auto DISABLED_ACTION_NAME = "always-disabled-action";
constexpr auto CLEAR_LIST_ACTION_NAME = "clear-recent-files";
constexpr auto REMOVE_ACTION_NAME = "remove-recent-file";

void recentManagerChangedCallback(GtkRecentManager* /*manager*/, RecentDocumentsSubmenu* recentDocsSubmenu) {
    recentDocsSubmenu->updateMenu();
}

void clearRecentFilesCallback(GSimpleAction*, GVariant*, gpointer) { RecentManager::clearRecentFiles(); }

auto createRecentMenuItem(const GtkRecentInfo* info, size_t i) {
    std::string display_name = gtk_recent_info_get_display_name(const_cast<GtkRecentInfo*>(info));

    // escape underscore
    StringUtils::replaceAllChars(display_name, {replace_pair('_', "__")});
    std::string label = FS(FORMAT_STR("{1}. {2}") % (i + 1) % display_name);

    std::string action = G_ACTION_NAMESPACE;
    action += OPEN_ACTION_NAME;
    action += "(uint64 ";
    action += std::to_string(i);
    action += ")";

    return xoj::util::GObjectSPtr<GMenuItem>(g_menu_item_new(label.c_str(), action.c_str()), xoj::util::adopt);
}

auto createRemoveMenuItem(const GtkRecentInfo* info, size_t i) {
    std::string display_name = gtk_recent_info_get_display_name(const_cast<GtkRecentInfo*>(info));

    // escape underscore
    StringUtils::replaceAllChars(display_name, {replace_pair('_', "__")});
    std::string label = FS(FORMAT_STR("Remove {1}") % display_name);

    std::string action = G_ACTION_NAMESPACE;
    action += REMOVE_ACTION_NAME;
    action += "(uint64 ";
    action += std::to_string(i);
    action += ")";

    GMenu* submenu = g_menu_new();
    g_menu_append(submenu, _("Remove from list"), action.c_str());
    
    return xoj::util::GObjectSPtr<GMenu>(submenu, xoj::util::adopt);

    // auto item = xoj::util::GObjectSPtr<GMenuItem>(g_menu_item_new(label.c_str(), action.c_str()), xoj::util::adopt);
    
    // // Create submenu for the remove action
    // GMenu* submenu = g_menu_new();
    // std::string remove_action = G_ACTION_NAMESPACE;
    // remove_action += REMOVE_ACTION_NAME;
    // remove_action += "(uint64 ";
    // remove_action += std::to_string(i);
    // remove_action += ")";
    // g_menu_append(submenu, _("Remove from list"), remove_action.c_str());
    // g_menu_item_set_submenu(item.get(), G_MENU_MODEL(submenu));
    
    // return item;

}

template <typename container>
auto createRecentMenu(const container& recentFiles, size_t start_index) -> xoj::util::GObjectSPtr<GMenu> {
    if (recentFiles.empty()) {
        return nullptr;
    }
    GMenu* menu = g_menu_new();

    for (auto& recent: recentFiles) {
        //g_menu_append_item(menu, createRecentMenuItem(recent.get(), start_index++).get());

        auto item = createRecentMenuItem(recent.get(), start_index);
        auto removeItem = createRemoveMenuItem(recent.get(), start_index);

        g_menu_item_set_submenu(item.get(), G_MENU_MODEL(removeItem.get()));
        g_menu_append_item(menu, item.get());

        // GMenu* submenu = g_menu_new();
        // g_menu_append_item(submenu, removeItem.get());
        // g_menu_item_set_submenu(item.get(), G_MENU_MODEL(submenu));
        // g_menu_append_item(menu, item.get());

        start_index++;
    }
    
    return xoj::util::GObjectSPtr<GMenu>(menu, xoj::util::adopt);
}

auto createEmptyListPlaceholder() {
    return xoj::util::GObjectSPtr<GMenuItem>(g_menu_item_new(_("No recent files"), DISABLED_ACTION_NAME),
                                             xoj::util::adopt);
}

auto createClearListSection() {
    // Todo(cpp20): constexpr this concatenation
    std::string action = G_ACTION_NAMESPACE;
    action += CLEAR_LIST_ACTION_NAME;

    GMenu* menu = g_menu_new();
    g_menu_append(menu, _("Clear list"), action.c_str());

    return xoj::util::GObjectSPtr<GMenu>(menu, xoj::util::adopt);
}
}  // namespace

RecentDocumentsSubmenu::RecentDocumentsSubmenu(Control* control, GtkApplicationWindow* win): control(control) {
    GtkRecentManager* recentManager = gtk_recent_manager_get_default();
    this->recentHandlerId = g_signal_connect(recentManager, "changed", G_CALLBACK(recentManagerChangedCallback), this);

    if (!win) {
        g_warning("RecentDocumentsSubmenu::RecentDocumentsSubmenu: no GtkApplicationWindow provided. Cannot push the "
                  "appropriate GAction.");
    } else {
        static_assert(is_action_namespace_match<decltype(win)>(G_ACTION_NAMESPACE));

        openFileAction.reset(g_simple_action_new(OPEN_ACTION_NAME, G_VARIANT_TYPE_UINT64), xoj::util::adopt);
        g_signal_connect(G_OBJECT(openFileAction.get()), "activate", G_CALLBACK(openFileCallback), this);
        g_action_map_add_action(G_ACTION_MAP(win), G_ACTION(openFileAction.get()));

        clearListAction.reset(g_simple_action_new(CLEAR_LIST_ACTION_NAME, nullptr), xoj::util::adopt);
        g_signal_connect(G_OBJECT(clearListAction.get()), "activate", G_CALLBACK(clearRecentFilesCallback), nullptr);
        g_action_map_add_action(G_ACTION_MAP(win), G_ACTION(clearListAction.get()));

        removeFileAction.reset(g_simple_action_new(REMOVE_ACTION_NAME, G_VARIANT_TYPE_UINT64), xoj::util::adopt);
        g_signal_connect(G_OBJECT(removeFileAction.get()), "activate", G_CALLBACK(removeFileCallback), this);
        g_action_map_add_action(G_ACTION_MAP(win), G_ACTION(removeFileAction.get()));
    }
}

RecentDocumentsSubmenu::~RecentDocumentsSubmenu() {
    if (this->recentHandlerId) {
        GtkRecentManager* recentManager = gtk_recent_manager_get_default();
        g_signal_handler_disconnect(recentManager, this->recentHandlerId);
        this->recentHandlerId = 0;
    }
}

void RecentDocumentsSubmenu::updateMenu() {
    auto fileList = RecentManager::getRecentFiles();

    xoppFiles.clear();
    std::transform(fileList.recentXoppFiles.begin(), fileList.recentXoppFiles.end(), std::back_inserter(xoppFiles),
                   [](auto& info) { return Util::fromUri(gtk_recent_info_get_uri(info.get())).value(); });
    pdfFiles.clear();
    std::transform(fileList.recentPdfFiles.begin(), fileList.recentPdfFiles.end(), std::back_inserter(pdfFiles),
                   [](auto& info) { return Util::fromUri(gtk_recent_info_get_uri(info.get())).value(); });

    this->menuXoppFiles = createRecentMenu(fileList.recentXoppFiles, 0);
    this->menuPdfFiles = createRecentMenu(fileList.recentPdfFiles, fileList.recentXoppFiles.size());

    xoj_assert(recentFilesSubmenu);
    g_menu_remove_all(recentFilesSubmenu.get());

    if (this->menuXoppFiles) {
        g_menu_append_section(recentFilesSubmenu.get(), nullptr, G_MENU_MODEL(this->menuXoppFiles.get()));
    }
    if (this->menuPdfFiles) {
        g_menu_append_section(recentFilesSubmenu.get(), nullptr, G_MENU_MODEL(this->menuPdfFiles.get()));
    }
    if (this->menuXoppFiles || this->menuPdfFiles) {
        g_menu_append_section(recentFilesSubmenu.get(), nullptr, G_MENU_MODEL(createClearListSection().get()));
    } else {
        g_menu_append_item(recentFilesSubmenu.get(), createEmptyListPlaceholder().get());
    }
}

void RecentDocumentsSubmenu::addToMenubar(Menubar& menubar) {
    recentFilesSubmenu.reset(menubar.get<GMenu>(SUBMENU_ID, [](auto* p) { return G_MENU(p); }), xoj::util::ref);
    updateMenu();
}

void RecentDocumentsSubmenu::setDisabled(bool disabled) {
    g_simple_action_set_enabled(openFileAction.get(), !disabled);
    g_simple_action_set_enabled(clearListAction.get(), !disabled);
}

void RecentDocumentsSubmenu::openFileCallback(GSimpleAction* ga, GVariant* parameter, RecentDocumentsSubmenu* self) {
    auto index = g_variant_get_uint64(parameter);
    auto& path =
            index < self->xoppFiles.size() ? self->xoppFiles[index] : self->pdfFiles[index - self->xoppFiles.size()];
    self->control->openFile(path);
}

void RecentDocumentsSubmenu::removeFileCallback(GSimpleAction* ga, GVariant* parameter, RecentDocumentsSubmenu* self) {
    auto index = g_variant_get_uint64(parameter);
    auto& path = index < self->xoppFiles.size() ? self->xoppFiles[index] : self->pdfFiles[index - self->xoppFiles.size()];
    RecentManager::removeRecentFileFilename(path);
    self->updateMenu();
}

// void RecentDocumentsSubmenu::removeFile(const fs::path& path, RecentDocumentsSubmenu* self) {
//     // auto remove_from_list = [&path](auto& list) {
//     //     auto it = std::find(list.begin(), list.end(), path);
//     //     if (it != list.end()) {
//     //         list.erase(it);
//     //     }
//     // };
//     self->remove_from_vector(self->xoppFiles, path);
//     self->remove_from_vector(self->pdfFiles, path);
// }

// void RecentDocumentsSubmenu::remove_from_vector(TinyVector<fs::path, RecentManager::MAX_RECENT>& vec, const fs::path& path) {
//     int index = -1;
//     for (size_t i = 0; i < vec.size(); ++i) {
//         if (vec[i] == path) {
//             index = i;
//             break;
//         }
//     }
//     if (index != -1) { // if path is found in vec
//         for (size_t i = index; i < vec.size() - 1; ++i) {
//             vec[i] = vec[i + 1];
//         }
//         vec.pop_back();
//     }
// }