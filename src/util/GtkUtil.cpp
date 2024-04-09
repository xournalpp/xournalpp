#include "util/GtkUtil.h"

#include <string>
#include <string_view>

#include <gtk/gtk.h>

#include "util/Assert.h"
#include "util/raii/GVariantSPtr.h"

namespace xoj::util::gtk {

static GAction* findAction(GtkActionable* w) {
    const char* name = gtk_actionable_get_action_name(w);
    if (!name) {
        g_warning("xoj::util::gtk::findAction: No action name set");
        return nullptr;
    }
    std::string_view namesv = name;
    size_t dotpos = namesv.find(".");
    if (dotpos == std::string_view::npos) {
        g_warning("xoj::util::gtk::findAction: Action name is not of the form \"namespace.name\": %s", name);
        return nullptr;
    }
    std::string groupname(namesv.substr(0, dotpos));
#if GTK_MAJOR_VERSION == 3
    GActionGroup* win = gtk_widget_get_action_group(GTK_WIDGET(w), groupname.c_str());
#else
    auto* win = GTK_APPLICATION_WINDOW(gtk_widget_get_ancestor(GTK_WIDGET(w), GTK_TYPE_APPLICATION_WINDOW));
#endif
    if (!win) {
        // Most likely the widget just got removed from the toplevel
        g_debug("xoj::util::gtk::findAction: could not find action group \"%s\"", groupname.data());
        return nullptr;
    }
    if (!G_IS_ACTION_MAP(win)) {
        g_warning("xoj::util::gtk::findAction: GActionGroup is not a GActionMap");
        return nullptr;
    }
    auto shortname = namesv.substr(dotpos + 1);
    return g_action_map_lookup_action(G_ACTION_MAP(win), shortname.data());
}

void setToggleButtonUnreleasable(GtkToggleButton* btn) {
    // "hierarchy-change" is emitted when the widget is added to/removed from a toplevel's descendance
    // We use this to connect to the suitable GAction signals once the widget has been added to the toolbar
    g_signal_connect(btn, "hierarchy-changed", G_CALLBACK(+[](GtkWidget* btn, GtkWidget*, gpointer) {
                         GAction* action = findAction(GTK_ACTIONABLE(btn));
                         if (!action) {
                             return;
                         }

                         g_signal_connect_object(
                                 btn, "toggled", G_CALLBACK(+[](GtkToggleButton* btn, gpointer a) {
                                     xoj::util::GVariantSPtr state(g_action_get_state(G_ACTION(a)), xoj::util::adopt);
                                     GVariant* target = gtk_actionable_get_action_target_value(GTK_ACTIONABLE(btn));
                                     if (bool active = g_variant_equal(state.get(), target);
                                         active && !gtk_toggle_button_get_active(btn)) {
                                         gtk_toggle_button_set_active(btn, true);
                                     }
                                 }),
                                 action, GConnectFlags(0));
                     }),
                     nullptr);
}

void setWidgetFollowActionEnabled(GtkWidget* w, GAction* a) {
    g_signal_connect_object(a, "notify::enabled", G_CALLBACK(+[](GObject* a, GParamSpec*, gpointer w) {
                                bool b = g_action_get_enabled(G_ACTION(a));
                                gtk_widget_set_sensitive(GTK_WIDGET(w), b);
                            }),
                            w, GConnectFlags(0));
    gtk_widget_set_sensitive(w, g_action_get_enabled(a));
}

#if GTK_MAJOR_VERSION == 3
void setRadioButtonActionName(GtkRadioButton* btn, const char* actionNamespace, const char* actionName) {
    // "hierarchy-change" is emitted when the widget is added to/removed from a toplevel's descendance
    // We use this to connect to the suitable GAction signals once the widget has been added to the toolbar
    struct Data {
        std::string actionNamespace;
        std::string actionName;
    };
    g_signal_connect_data(
            btn, "hierarchy-changed", G_CALLBACK(+[](GtkWidget* btn, GtkWidget*, gpointer d) {
                Data* data = static_cast<Data*>(d);
                GActionGroup* win = gtk_widget_get_action_group(btn, data->actionNamespace.c_str());
                if (!win) {
                    // Most likely the widget just got removed from the toplevel
                    return;
                }
                xoj_assert(G_IS_ACTION_MAP(win));
                auto* action = g_action_map_lookup_action(G_ACTION_MAP(win), data->actionName.c_str());
                if (!action) {
                    g_warning("Could not find action \"win.%s\"", data->actionName.c_str());
                    return;
                }

                {
                    // btn owns the return GVariant of gtk_actionable_get_action_target_value()
                    GVariant* target = gtk_actionable_get_action_target_value(GTK_ACTIONABLE(btn));
                    // action does not own the return GVariant and it is not floating either!
                    xoj::util::GVariantSPtr state(g_action_get_state(G_ACTION(action)), xoj::util::adopt);
                    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(btn), g_variant_equal(target, state.get()));
                }

                static auto toggledCallback = +[](GtkToggleButton* btn, gpointer action) {
                    GVariant* tgt = gtk_actionable_get_action_target_value(GTK_ACTIONABLE(btn));
                    xoj_assert(tgt);
                    if (gtk_toggle_button_get_active(btn)) {
                        g_action_change_state(G_ACTION(action),
                                              gtk_actionable_get_action_target_value(GTK_ACTIONABLE(btn)));
                    }
                };
                g_signal_connect_object(btn, "toggled", G_CALLBACK(toggledCallback), action, GConnectFlags(0));

                g_signal_connect_object(
                        action, "notify::state", G_CALLBACK(+[](GObject* action, GParamSpec*, gpointer btn) {
                            // btn owns the return GVariant of gtk_actionable_get_action_target_value()
                            GVariant* target = gtk_actionable_get_action_target_value(GTK_ACTIONABLE(btn));
                            // action does not own the return GVariant and it is not floating either!
                            xoj::util::GVariantSPtr state(g_action_get_state(G_ACTION(action)), xoj::util::adopt);
                            xoj_assert(target);
                            xoj_assert(state);
                            if (g_variant_equal(target, state.get()) &&
                                !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(btn))) {
                                g_signal_handlers_block_by_func(btn, (gpointer)toggledCallback, action);
                                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(btn), true);
                                g_signal_handlers_unblock_by_func(btn, (gpointer)toggledCallback, action);
                            }
                        }),
                        btn, GConnectFlags(0));

                setWidgetFollowActionEnabled(btn, action);
            }),
            new Data{actionNamespace, actionName}, +[](gpointer d, GClosure*) { delete static_cast<Data*>(d); },
            GConnectFlags(0));
}
#endif
};  // namespace xoj::util::gtk
