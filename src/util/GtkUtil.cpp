#include "util/GtkUtil.h"

#include <string_view>

#include <gtk/gtk.h>

#include "util/Assert.h"
#include "util/raii/GVariantSPtr.h"

namespace xoj::util::gtk {

void setToggleButtonUnreleasable(GtkToggleButton* btn) {
    // "hierarchy-change" is emitted when the widget is added to/removed from a toplevel's descendance
    // We use this to connect to the suitable GAction signals once the widget has been added to the toolbar
    g_signal_connect(
            btn, "hierarchy-changed", G_CALLBACK(+[](GtkWidget* btn, GtkWidget*, gpointer) {
                const char* name = gtk_actionable_get_action_name(GTK_ACTIONABLE(btn));
                if (!name) {
                    g_warning("xoj::util::gtk::MakeToggleButtonUnreleasable callback: No action name set");
                    return;
                }
                std::string_view namesv = name;
                size_t dotpos = namesv.find(".");
                if (dotpos == std::string_view::npos) {
                    g_warning("xoj::util::gtk::MakeToggleButtonUnreleasable callback: Action name is not of the form "
                              "\"namespace.name\": %s",
                              name);
                    return;
                }
                std::string groupname(namesv.substr(0, dotpos));
                GActionGroup* win = gtk_widget_get_action_group(btn, groupname.c_str());
                if (!win) {
                    // Most likely the widget just got removed from the toplevel
                    g_debug("xoj::util::gtk::MakeToggleButtonUnreleasable callback: could not find action group \"%s\"",
                            groupname.data());
                    return;
                }
                if (!G_IS_ACTION_MAP(win)) {
                    g_warning(
                            "xoj::util::gtk::MakeToggleButtonUnreleasable callback: GActionGroup is not a GActionMap");
                    return;
                }
                auto shortname = namesv.substr(dotpos + 1);
                auto* action = g_action_map_lookup_action(G_ACTION_MAP(win), shortname.data());
                if (!action) {
                    g_warning("xoj::util::gtk::MakeToggleButtonUnreleasable callback: Could not find action "
                              "\"%s\". Looked for \"%s.%s\"",
                              name, groupname.data(), shortname.data());
                    return;
                }

                g_signal_connect_object(
                        btn, "toggled", G_CALLBACK(+[](GtkToggleButton* btn, gpointer a) {
                            xoj::util::GVariantSPtr state(g_action_get_state(G_ACTION(a)), xoj::util::adopt);
                            GVariant* target = gtk_actionable_get_action_target_value(GTK_ACTIONABLE(btn));
                            gtk_toggle_button_set_active(btn, g_variant_equal(state.get(), target));
                        }),
                        action, GConnectFlags(0));
            }),
            nullptr);
}
};  // namespace xoj::util::gtk
