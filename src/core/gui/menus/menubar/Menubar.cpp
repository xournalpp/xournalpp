#include "Menubar.h"

#include "control/Control.h"
#include "gui/GladeSearchpath.h"
#include "gui/MainWindow.h"
#include "util/XojMsgBox.h"
#include "util/i18n.h"
#include "util/raii/GVariantSPtr.h"

#include "PageTypeSubmenu.h"
#include "PluginsSubmenu.h"
#include "RecentDocumentsSubmenu.h"
#include "ToolbarSelectionSubmenu.h"
#include "config-features.h"  // for ENABLE_PLUGINS

constexpr auto MENU_XML_FILE = "mainmenubar.xml";
constexpr auto MENU_ID = "menubar";

Menubar::Menubar() = default;
Menubar::~Menubar() noexcept = default;

/// @brief Recursively removes all items with attribute "class" with value classname
static void removeItemsWithClass(GMenu* menu, const char* classname) {
    int N = g_menu_model_get_n_items(G_MENU_MODEL(menu));
    for (int n = 0; n < N; ++n) {
        xoj::util::GVariantSPtr c(
                g_menu_model_get_item_attribute_value(G_MENU_MODEL(menu), n, "class", G_VARIANT_TYPE_STRING),
                xoj::util::adopt);
        if (c && std::string_view(g_variant_get_string(c.get(), nullptr)) == classname) {
            g_menu_remove(menu, n--);
            N--;
        } else {
            xoj::util::GObjectSPtr<GMenuLinkIter> it(g_menu_model_iterate_item_links(G_MENU_MODEL(menu), n),
                                                     xoj::util::adopt);
            while (g_menu_link_iter_next(it.get())) {
                xoj::util::GObjectSPtr<GMenuModel> sub(g_menu_link_iter_get_value(it.get()), xoj::util::adopt);
                if (sub && G_IS_MENU(sub.get())) {
                    removeItemsWithClass(G_MENU(sub.get()), classname);
                }
            }
        }
    }
}

void Menubar::populate(const GladeSearchpath* gladeSearchPath, MainWindow* win) {
    builder.reset(gtk_builder_new(), xoj::util::adopt);

    auto filepath = gladeSearchPath->findFile("", MENU_XML_FILE);
    GError* error = nullptr;

    if (!gtk_builder_add_from_file(builder.get(), filepath.u8string().c_str(), &error)) {
        std::string msg = FS(_F("Error loading menubar XML file (try to load \"{1}\")") % filepath.u8string());

        if (error != nullptr) {
            msg += "\n";
            msg += error->message;
            g_error_free(error);
        }
        XojMsgBox::showErrorToUser(nullptr, msg);
        return;
    }

    menu = G_MENU_MODEL(gtk_builder_get_object(builder.get(), MENU_ID));

    Control* ctrl = win->getControl();

    recentDocumentsSubmenu = std::make_unique<RecentDocumentsSubmenu>(ctrl, GTK_APPLICATION_WINDOW(win->getWindow()));
    toolbarSelectionSubmenu =
            std::make_unique<ToolbarSelectionSubmenu>(win, ctrl->getSettings(), win->getToolMenuHandler());
    pageTypeSubmenu = std::make_unique<PageTypeSubmenu>(ctrl->getPageTypes(), ctrl->getPageBackgroundChangeController(),
                                                        ctrl->getSettings(), GTK_APPLICATION_WINDOW(win->getWindow()));
#ifdef ENABLE_PLUGINS
    pluginsSubmenu =
            std::make_unique<PluginsSubmenu>(ctrl->getPluginController(), GTK_APPLICATION_WINDOW(win->getWindow()));
#endif

    forEachSubmenu([&](auto& subm) { subm.addToMenubar(*this); });

    if (!ctrl->getAudioController()) {
        removeItemsWithClass(G_MENU(menu), "audio");
    }
}

void Menubar::setDisabled(bool disabled) {
    forEachSubmenu([&](auto& subm) { subm.setDisabled(disabled); });
}
