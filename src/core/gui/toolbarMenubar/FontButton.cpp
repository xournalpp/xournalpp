#include "FontButton.h"

#include <sstream>  // for stringstream
#include <utility>  // for move

#include <glib-object.h>  // for g_object_ref, g_obj...

#include "control/Control.h"                      // for Control
#include "control/actions/ActionDatabase.h"       // for ActionDatabase
#include "gui/toolbarMenubar/AbstractToolItem.h"  // for AbstractToolItem
#include "model/Font.h"                           // for Font
#include "util/GtkUtil.h"                         // for fixActionableInitialSensitivity
#include "util/gtk4_helper.h"                     // for gtk_button_set_child
#include "util/i18n.h"                            // for _
#include "util/raii/GVariantSPtr.h"
#include "util/serdesstream.h"


FontButton::FontButton(std::string id, ActionDatabase& db):
        AbstractToolItem(std::move(id), Category::TOOLS), gAction(db.getAction(Action::FONT)) {}

static GtkWidget* makeChild(const char* desc) {
    XojFont font(desc);
    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_margin_start(box, 4);
    gtk_widget_set_margin_end(box, 4);
    GtkWidget* label = gtk_label_new(nullptr);
    auto markup = "<span font=\"" + font.getName() + "\">" + font.getName() + "</span>";
    gtk_label_set_markup(GTK_LABEL(label), markup.c_str());
    gtk_box_append(GTK_BOX(box), label);

    auto size = serdes_stream<std::stringstream>();
    size << font.getSize();
    gtk_box_append(GTK_BOX(box), gtk_label_new(size.str().c_str()));
    gtk_widget_show_all(box);
    return box;
}

auto FontButton::createItem(bool horizontal) -> xoj::util::WidgetSPtr {
    GtkWidget* btn = gtk_button_new();
    gtk_widget_set_can_focus(btn, false);  // todo(gtk4) not necessary anymore
    xoj::util::GVariantSPtr font(g_action_get_state(G_ACTION(gAction.get())), xoj::util::adopt);
    const char* desc = g_variant_get_string(font.get(), nullptr);
    gtk_button_set_child(GTK_BUTTON(btn), makeChild(desc));
    gtk_widget_set_tooltip_text(btn, getToolDisplayName().c_str());
    gtk_actionable_set_action_name(GTK_ACTIONABLE(btn),
                                   (std::string("win.") + Action_toString(Action::SELECT_FONT)).c_str());

    g_signal_connect_object(gAction.get(), "notify::state", G_CALLBACK(+[](GObject* action, GParamSpec*, gpointer btn) {
                                xoj::util::GVariantSPtr font(g_action_get_state(G_ACTION(action)), xoj::util::adopt);
                                const char* desc = g_variant_get_string(font.get(), nullptr);
                                gtk_button_set_child(GTK_BUTTON(btn), makeChild(desc));
                            }),
                            btn, GConnectFlags(0));

    GtkToolItem* it = gtk_tool_item_new();
    gtk_container_add(GTK_CONTAINER(it), btn);
    /// Makes a proxy item for the toolbar's overflow menu
    auto createProxy = [this]() {
        GtkWidget* proxy = gtk_menu_item_new();
        auto* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
        gtk_container_add(GTK_CONTAINER(proxy), box);
        gtk_box_append(GTK_BOX(box), getNewToolIcon());
        gtk_box_append(GTK_BOX(box), gtk_label_new(getToolDisplayName().c_str()));
        gtk_actionable_set_action_name(GTK_ACTIONABLE(proxy),
                                       (std::string("win.") + Action_toString(Action::SELECT_FONT)).c_str());
        xoj::util::gtk::fixActionableInitialSensitivity(GTK_ACTIONABLE(proxy));
        return proxy;
    };
    gtk_tool_item_set_proxy_menu_item(it, "", createProxy());
    return xoj::util::WidgetSPtr(GTK_WIDGET(it), xoj::util::adopt);
}

auto FontButton::getToolDisplayName() const -> std::string { return _("Font"); }

auto FontButton::getNewToolIcon() const -> GtkWidget* {
    return gtk_image_new_from_icon_name("font-x-generic", GTK_ICON_SIZE_LARGE_TOOLBAR);
}
