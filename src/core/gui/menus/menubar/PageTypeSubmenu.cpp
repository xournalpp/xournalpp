#include "PageTypeSubmenu.h"

#include <memory>  // for unique_ptr
#include <optional>
#include <string>  // for string
#include <vector>

#include <glib-object.h>  // for G_CALLBACK, g_sig...

#include "control/PageBackgroundChangeController.h"
#include "control/pagetype/PageTypeHandler.h"  // for PageTypeInfo, Pag...
#include "gui/menus/StaticAssertActionNamespace.h"
#include "util/i18n.h"  // for _
#include "util/raii/GVariantSPtr.h"

#include "Menubar.h"

namespace {
static constexpr auto G_ACTION_NAMESPACE = "win.";
static constexpr auto SELECTION_ACTION_NAME = "menu.pick-page-type";
static constexpr auto APPLY_ALL_ACTION_NAME = "menu.apply-current-page-type-to-all-pages";
static constexpr auto SUBMENU_ID = "menuJournalPaperBackground";

auto createPageTypesSection(const std::vector<std::unique_ptr<PageTypeInfo>>& pageTypes, size_t index) {
    GMenu* menu = g_menu_new();

    // Todo(c++20) constexpr this
    std::string actionName = G_ACTION_NAMESPACE;
    actionName += SELECTION_ACTION_NAME;

    for (auto& pageInfo: pageTypes) {
        xoj::util::GObjectSPtr<GMenuItem> item(g_menu_item_new(pageInfo->name.c_str(), nullptr), xoj::util::adopt);
        g_menu_item_set_action_and_target_value(item.get(), actionName.c_str(),
                                                xoj::util::makeGVariantSPtr(index++).get());
        g_menu_append_item(menu, item.get());
    }
    return xoj::util::GObjectSPtr<GMenu>(menu, xoj::util::adopt);
}

auto createApplyToAllPagesSection() {
    GMenu* menu = g_menu_new();

    // Todo(c++20) constexpr this
    std::string actionName = G_ACTION_NAMESPACE;
    actionName += APPLY_ALL_ACTION_NAME;

    g_menu_append(menu, _("Apply to all pages"), actionName.c_str());
    return xoj::util::GObjectSPtr<GMenu>(menu, xoj::util::adopt);
}
};  // namespace


PageTypeSubmenu::PageTypeSubmenu(PageTypeHandler* typesHandler, PageBackgroundChangeController* controller,
                                 const Settings* settings, GtkApplicationWindow* win):
        PageTypeSelectionMenuBase(typesHandler, settings, SELECTION_ACTION_NAME),
        controller(controller),
        generatedPageTypesSection(createPageTypesSection(typesHandler->getPageTypes(), 0)),
        specialPageTypesSection(
                createPageTypesSection(typesHandler->getSpecialPageTypes(), typesHandler->getPageTypes().size())),
        applyToAllPagesSection(createApplyToAllPagesSection()),
        applyToAllPagesAction(g_simple_action_new(APPLY_ALL_ACTION_NAME, nullptr), xoj::util::adopt) {

    this->changeCurrentPageUponCallback = true;

    static_assert(is_action_namespace_match<decltype(win)>(G_ACTION_NAMESPACE));

    g_action_map_add_action(G_ACTION_MAP(win), G_ACTION(typeSelectionAction.get()));
    g_action_map_add_action(G_ACTION_MAP(win), G_ACTION(applyToAllPagesAction.get()));

    g_signal_connect(G_OBJECT(applyToAllPagesAction.get()), "activate",
                     G_CALLBACK(+[](GSimpleAction*, GVariant*, PageBackgroundChangeController* controller) {
                         controller->applyCurrentPageBackgroundToAll();
                     }),
                     this->controller);
}

void PageTypeSubmenu::setDisabled(bool disabled) {
    g_simple_action_set_enabled(typeSelectionAction.get(), !disabled);
    g_simple_action_set_enabled(applyToAllPagesAction.get(), !disabled);
}

void PageTypeSubmenu::addToMenubar(Menubar& menubar) {
    GMenu* submenu = menubar.get<GMenu>(SUBMENU_ID, [](auto* p) { return G_MENU(p); });
    g_menu_append_section(submenu, nullptr, G_MENU_MODEL(generatedPageTypesSection.get()));
    g_menu_append_section(submenu, nullptr, G_MENU_MODEL(specialPageTypesSection.get()));
    g_menu_append_section(submenu, nullptr, G_MENU_MODEL(applyToAllPagesSection.get()));
}

void PageTypeSubmenu::entrySelected(const PageTypeInfo*) {
    if (this->selectedPT && this->controller) {
        this->controller->changeCurrentPageBackground(this->selectedPT.value());
    }
}
