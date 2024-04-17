#include "Menubar.h"

#include "control/Control.h"
#include "gui/GladeSearchpath.h"
#include "gui/MainWindow.h"
#include "util/Assert.h"
#include "util/XojMsgBox.h"
#include "util/i18n.h"
#include "util/raii/CStringWrapper.h"
#include "util/raii/GVariantSPtr.h"

#include "PageTypeSubmenu.h"
#include "PluginsSubmenu.h"
#include "RecentDocumentsSubmenu.h"
#include "ToolbarSelectionSubmenu.h"
#include "config-features.h"  // for ENABLE_PLUGINS

constexpr auto MENU_XML_FILE = "mainmenubar.ui";
constexpr auto MENU_ID = "menubar";
constexpr auto UNDO_REDO_SECTION_ID = "sectionUndoRedo";

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

static void setupAccels(GMenu* menu, GtkApplication* app) {
    auto getAttr = [&](int itemNb, const char* attr, const GVariantType* type) {
        return xoj::util::GVariantSPtr(g_menu_model_get_item_attribute_value(G_MENU_MODEL(menu), itemNb, attr, type),
                                       xoj::util::adopt);
    };

    int N = g_menu_model_get_n_items(G_MENU_MODEL(menu));
    for (int n = 0; n < N; ++n) {
        auto acc = getAttr(n, "accel", G_VARIANT_TYPE_STRING);
        if (acc) {
            auto act = getAttr(n, "action", G_VARIANT_TYPE_STRING);
            xoj_assert_message(act,
                               std::string("Menu entry without linked action: ") +
                                       g_variant_get_string(getAttr(n, "label", G_VARIANT_TYPE_STRING).get(), nullptr));

            const char* a[2] = {g_variant_get_string(acc.get(), nullptr), nullptr};

            if (auto tgt = getAttr(n, "target", nullptr)) {
                auto tgtstr = xoj::util::OwnedCString::assumeOwnership(g_variant_print(tgt.get(), true));
                auto fullActionName = std::string(g_variant_get_string(act.get(), nullptr)) + "(" + tgtstr.get() + ")";
                gtk_application_set_accels_for_action(app, fullActionName.c_str(), a);
            } else {
                gtk_application_set_accels_for_action(app, g_variant_get_string(act.get(), nullptr), a);
            }
        }

        xoj::util::GObjectSPtr<GMenuLinkIter> it(g_menu_model_iterate_item_links(G_MENU_MODEL(menu), n),
                                                 xoj::util::adopt);
        while (g_menu_link_iter_next(it.get())) {
            xoj::util::GObjectSPtr<GMenuModel> sub(g_menu_link_iter_get_value(it.get()), xoj::util::adopt);
            if (sub && G_IS_MENU(sub.get())) {
                setupAccels(G_MENU(sub.get()), app);
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

    undoRedoSection = G_MENU(gtk_builder_get_object(builder.get(), UNDO_REDO_SECTION_ID));

    gtk_application_set_menubar(gtk_window_get_application(GTK_WINDOW(win->getWindow())), menu);
    gtk_window_set_handle_menubar_accel(GTK_WINDOW(win->getWindow()), false);  // Disable GTK handling F10.

    /*
     * Due to a bug in gtk4 see https://gitlab.gnome.org/GNOME/gtk/-/issues/4574
     *                          https://gitlab.gnome.org/GNOME/gtk/-/issues/4607
     * The accelerators from the menu ui file are not set up correctly by gtk_application_set_menubar()
     *      (in fact, they work when the corresponding menu is shown...)
     * Set them up by hand here.
     */
    setupAccels(G_MENU(menu), gtk_window_get_application(GTK_WINDOW(win->getWindow())));
    gtk_application_window_set_show_menubar(GTK_APPLICATION_WINDOW(win->getWindow()), true);

    /**** Debug info ****/
    // char** act = gtk_application_list_action_descriptions(gtk_window_get_application(GTK_WINDOW(win->getWindow())));
    // for (char** it = act; *it != nullptr; it++) {
    //     printf("%s : ", *it);
    //     char** acc = gtk_application_get_accels_for_action(gtk_window_get_application(GTK_WINDOW(win->getWindow())),
    //     *it); for (char** it = acc; *it != nullptr; it++) {
    //         printf("%s ", *it);
    //     }
    //     g_strfreev(acc);
    //     printf("\n");
    // }
    // g_strfreev(act);
}

void Menubar::setUndoDescription(const std::string& description) {
    xoj_assert(undoRedoSection);
    g_menu_remove(undoRedoSection, 0);
    g_menu_prepend(undoRedoSection, description.c_str(), "win.undo");
}

void Menubar::setRedoDescription(const std::string& description) {
    xoj_assert(undoRedoSection);
    g_menu_remove(undoRedoSection, 1);
    g_menu_append(undoRedoSection, description.c_str(), "win.redo");
}

void Menubar::setDisabled(bool disabled) {
    forEachSubmenu([&](auto& subm) { subm.setDisabled(disabled); });
}
