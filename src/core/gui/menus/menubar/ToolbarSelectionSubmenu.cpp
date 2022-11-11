#include "ToolbarSelectionSubmenu.h"

#include <algorithm>
#include <string>  // for operator==, string

#include <glib-object.h>  // for G_CALLBACK, g_sig...

#include "control/settings/Settings.h"  // for Settings
#include "gui/MainWindow.h"             // for MainWindow
#include "gui/menus/StaticAssertActionNamespace.h"
#include "gui/toolbarMenubar/ToolMenuHandler.h"     // for ToolMenuHandler
#include "gui/toolbarMenubar/model/ToolbarData.h"   // for ToolbarData
#include "gui/toolbarMenubar/model/ToolbarModel.h"  // for ToolbarModel

#include "Menubar.h"

namespace {
constexpr auto G_ACTION_NAMESPACE = "win.";
constexpr auto G_ACTION_NAME = "select-toolbar";
/// id from ui/mainmenubar.xml
constexpr auto SUBMENU_ID = "menuViewToolbar";

auto createToolbarSelectionMenuItem(const ToolbarData* toolbarData) {

    std::string action = G_ACTION_NAMESPACE;
    action += G_ACTION_NAME;
    action += "('";
    action += toolbarData->getId();
    action += "')";

    return xoj::util::GObjectSPtr<GMenuItem>(g_menu_item_new(toolbarData->getName().c_str(), action.c_str()),
                                             xoj::util::adopt);
}

void toolbarSelectionMenuChangeStateCallback(GSimpleAction* ga, GVariant* parameter, MainWindow* window) {
    g_simple_action_set_state(ga, parameter);

    size_t l = 0;
    const char* p = g_variant_get_string(parameter, &l);
    std::string toolbarId(p, l);

    window->toolbarSelected(toolbarId);
}
};  // namespace

ToolbarSelectionSubmenu::ToolbarSelectionSubmenu(MainWindow* win, Settings* settings, ToolMenuHandler* toolbar):
        gAction(g_simple_action_new_stateful(G_ACTION_NAME, G_VARIANT_TYPE_STRING,
                                             g_variant_new_string(settings->getSelectedToolbar().c_str())),
                xoj::util::adopt) {
    const auto& toolbars = *toolbar->getModel()->getToolbars();

    auto it = toolbars.begin();
    this->stockConfigurationsSection.reset(g_menu_new(), xoj::util::adopt);
    for (; it != toolbars.end() && (*it)->isPredefined(); ++it) {
        g_menu_append_item(this->stockConfigurationsSection.get(), createToolbarSelectionMenuItem(*it).get());
    }
    this->customConfigurationsSection.reset(g_menu_new(), xoj::util::adopt);
    for (; it != toolbars.end(); ++it) {
        g_menu_append_item(this->customConfigurationsSection.get(), createToolbarSelectionMenuItem(*it).get());
    }

    g_signal_connect(G_OBJECT(gAction.get()), "change-state", G_CALLBACK(toolbarSelectionMenuChangeStateCallback), win);
    static_assert(is_action_namespace_match<decltype(win)>(G_ACTION_NAMESPACE));
    g_action_map_add_action(G_ACTION_MAP(win->getWindow()), G_ACTION(gAction.get()));
}

ToolbarSelectionSubmenu::~ToolbarSelectionSubmenu() = default;

void ToolbarSelectionSubmenu::setDisabled(bool disabled) { g_simple_action_set_enabled(gAction.get(), !disabled); }

void ToolbarSelectionSubmenu::addToMenubar(Menubar& menubar) {
    GMenu* submenu = menubar.get<GMenu>(SUBMENU_ID, [](auto* p) { return G_MENU(p); });
    g_menu_prepend_section(submenu, nullptr, G_MENU_MODEL(customConfigurationsSection.get()));
    g_menu_prepend_section(submenu, nullptr, G_MENU_MODEL(stockConfigurationsSection.get()));
}

void ToolbarSelectionSubmenu::update(ToolMenuHandler* toolbarHandler, const ToolbarData* selectedToolbar) {
    const auto& toolbars = *toolbarHandler->getModel()->getToolbars();
    // The first half stockConfigurationsSection of the menu has already been generated: fast forward to the first
    // custom config
    auto it =
            std::find_if_not(toolbars.begin(), toolbars.end(), [](const ToolbarData* d) { return d->isPredefined(); });
    this->customConfigurationsSection.reset(g_menu_new(), xoj::util::adopt);
    for (; it != toolbars.end(); ++it) {
        g_menu_append_item(this->customConfigurationsSection.get(), createToolbarSelectionMenuItem(*it).get());
    }
    // Does not fire a "change-state" signal
    g_simple_action_set_state(gAction.get(), g_variant_new_string(selectedToolbar->getId().c_str()));
}
