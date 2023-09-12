#include "ColorSelectorToolItem.h"

#include <gtk/gtk.h>

#include "control/actions/ActionDatabase.h"
#include "gui/toolbarMenubar/icon/ColorIcon.h"  // for ColorIcon
#include "util/GtkUtil.h"                       // for fixActionableInitialSensitivity
#include "util/gtk4_helper.h"                   // for gtk_button_set_child
#include "util/i18n.h"                          // for _

constexpr int ICON_SIZE = 22;

ColorSelectorToolItem::ColorSelectorToolItem(ActionDatabase& db):
        AbstractToolItem("COLOR_SELECT", Category::COLORS), gAction(db.getAction(Action::TOOL_COLOR)) {}

auto ColorSelectorToolItem::createItem(bool) -> xoj::util::WidgetSPtr {
    GtkWidget* btn = gtk_button_new();
    gtk_widget_set_can_focus(btn, false);  // todo(gtk4) not necessary anymore
    xoj::util::GVariantSPtr v(g_action_get_state(G_ACTION(gAction.get())), xoj::util::adopt);
    Color initialColor = getGVariantValue<Color>(v.get());
    gtk_button_set_child(GTK_BUTTON(btn), ColorIcon::newGtkImage(initialColor, ICON_SIZE, false));

    gtk_widget_set_tooltip_text(btn, this->getToolDisplayName().c_str());
    gtk_widget_set_sensitive(btn, g_action_get_enabled(G_ACTION(gAction.get())));
    gtk_actionable_set_action_name(GTK_ACTIONABLE(btn),
                                   (std::string("win.") + Action_toString(Action::SELECT_COLOR)).c_str());

    // The color follows the value of the GAction state
    // NB: we use the notify::state signal because "change-state" is not raised by g_simple_action_set_state()
    g_signal_connect_object(gAction.get(), "notify::state", G_CALLBACK(+[](GObject* a, GParamSpec*, gpointer btn) {
                                xoj::util::GVariantSPtr state(g_action_get_state(G_ACTION(a)), xoj::util::adopt);
                                auto c = getGVariantValue<Color>(state.get());
                                gtk_button_set_child(GTK_BUTTON(btn), ColorIcon::newGtkImage(c, ICON_SIZE, false));
                            }),
                            btn, GConnectFlags(0));
    return xoj::util::WidgetSPtr(btn, xoj::util::adopt);
}

auto ColorSelectorToolItem::getToolDisplayName() const -> std::string { return _("Select color"); }

auto ColorSelectorToolItem::getNewToolIcon() const -> GtkWidget* {
    return ColorIcon::newGtkImage(Colors::gray, ICON_SIZE, false);
}
