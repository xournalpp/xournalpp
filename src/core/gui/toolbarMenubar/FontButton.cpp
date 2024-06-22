#include "FontButton.h"

#include <sstream>  // for stringstream
#include <utility>  // for move

#include <glib-object.h>  // for g_object_ref, g_obj...

#include "control/Control.h"                      // for Control
#include "control/actions/ActionDatabase.h"       // for ActionDatabase
#include "model/Font.h"                           // for Font
#include "util/GtkUtil.h"                         // for fixActionableInitialSensitivity
#include "util/i18n.h"                            // for _
#include "util/raii/GVariantSPtr.h"
#include "util/serdesstream.h"


FontButton::FontButton(std::string id, ActionDatabase& db):
        ItemWithNamedIcon(std::move(id), Category::TOOLS), gAction(db.getAction(Action::FONT)) {}

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
    return box;
}

auto FontButton::createItem(ToolbarSide side) -> Widgetry {
    GtkWidget* btn = gtk_button_new();
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

    auto createProxy = [this]() {
        GtkWidget* proxy = gtk_button_new();
        auto* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
        gtk_button_set_child(GTK_BUTTON(proxy), box);
        gtk_box_append(GTK_BOX(box), getNewToolIcon());
        gtk_box_append(GTK_BOX(box), gtk_label_new(getToolDisplayName().c_str()));
        gtk_actionable_set_action_name(GTK_ACTIONABLE(proxy),
                                       (std::string("win.") + Action_toString(Action::SELECT_FONT)).c_str());
        return proxy;
    };


    return {xoj::util::WidgetSPtr(btn, xoj::util::adopt), xoj::util::WidgetSPtr(createProxy(), xoj::util::adopt)};
}

auto FontButton::getToolDisplayName() const -> std::string { return _("Font"); }

auto FontButton::getIconName() const -> const char* { return "font-x-generic"; }
