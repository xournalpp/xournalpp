#include "RecentDocumentsSubmenu.h"

#include <string>  // for string, allocator, operator+

#include <gio/gio.h>  // for GMenu

#include "control/Control.h"
#include "control/RecentManager.h"
#include "gui/MainWindow.h"
#include "gui/menus/StaticAssertActionNamespace.h"
#include "util/PathUtil.h"           // for fromUri, toUri, hasPdfFileExt
#include "util/PlaceholderString.h"  // for PlaceholderString
#include "util/StringUtils.h"        // for replace_pair, StringUtils
#include "util/TinyVector.h"         // for TinyVector
#include "util/i18n.h"               // for FS, FORMAT_STR, C_F

namespace {

void recentManagerChangedCallback(GtkRecentManager* /*manager*/, RecentDocumentsSubmenu* recentDocsSubmenu) {
    recentDocsSubmenu->updateMenu();
}

void clearRecentFilesCallback(GSimpleAction*, GVariant*, gpointer) { RecentManager::clearRecentFiles(); }

auto createRecentMenuItem(const GtkRecentInfo* info, size_t i) {
    std::string display_name = gtk_recent_info_get_display_name(const_cast<GtkRecentInfo*>(info));

    // escape underscore
    StringUtils::replaceAllChars(display_name, {replace_pair('_', "__")});
    std::string label = FS(FORMAT_STR("{1}. {2}") % (i + 1) % display_name);

    std::string action = RecentDocumentsSubmenu::G_ACTION_NAMESPACE;
    action += RecentDocumentsSubmenu::OPEN_ACTION_NAME;
    action += "(uint64 ";
    action += std::to_string(i);
    action += ")";

    return xoj::util::GObjectSPtr<GMenuItem>(g_menu_item_new(label.c_str(), action.c_str()), xoj::util::adopt);
}

template <typename container>
auto createRecentMenu(const container& recentFiles, size_t start_index) -> xoj::util::GObjectSPtr<GMenu> {
    if (recentFiles.empty()) {
        return nullptr;
    }
    GMenu* menu = g_menu_new();

    for (auto& recent: recentFiles) {
        g_menu_append_item(menu, createRecentMenuItem(recent.get(), start_index++).get());
    }
    return xoj::util::GObjectSPtr<GMenu>(menu, xoj::util::adopt);
}

auto createEmptyListPlaceholder() {
    return xoj::util::GObjectSPtr<GMenuItem>(
            g_menu_item_new(_("No recent files"), RecentDocumentsSubmenu::DISABLED_ACTION_NAME), xoj::util::adopt);
}

auto createClearListSection() {
    // Todo(cpp20): constexpr this concatenation
    std::string action = RecentDocumentsSubmenu::G_ACTION_NAMESPACE;
    action += RecentDocumentsSubmenu::CLEAR_LIST_ACTION_NAME;

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

    xoj::util::GObjectSPtr<GMenu> submenu(g_menu_new(), xoj::util::adopt);
    if (this->menuXoppFiles) {
        g_menu_append_section(submenu.get(), nullptr, G_MENU_MODEL(this->menuXoppFiles.get()));
    }
    if (this->menuPdfFiles) {
        g_menu_append_section(submenu.get(), nullptr, G_MENU_MODEL(this->menuPdfFiles.get()));
    }
    if (this->menuXoppFiles || this->menuPdfFiles) {
        g_menu_append_section(submenu.get(), nullptr, G_MENU_MODEL(createClearListSection().get()));
    } else {
        g_menu_append_item(submenu.get(), createEmptyListPlaceholder().get());
    }
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(this->menuItem.get()),
                              gtk_menu_new_from_model(G_MENU_MODEL(submenu.get())));
}

void RecentDocumentsSubmenu::addToMenubar(MainWindow* win) {
    this->menuItem.reset(win->get(SUBMENU_ID), xoj::util::ref);
    updateMenu();
}

void RecentDocumentsSubmenu::setDisabled(bool disabled) {
    g_simple_action_set_enabled(openFileAction.get(), !disabled);
    g_simple_action_set_enabled(clearListAction.get(), !disabled);
    gtk_widget_set_sensitive(menuItem.get(), !disabled);
}

void RecentDocumentsSubmenu::openFileCallback(GSimpleAction* ga, GVariant* parameter, RecentDocumentsSubmenu* self) {
    auto index = g_variant_get_uint64(parameter);
    auto& path =
            index < self->xoppFiles.size() ? self->xoppFiles[index] : self->pdfFiles[index - self->xoppFiles.size()];
    self->control->openFile(path);
}
