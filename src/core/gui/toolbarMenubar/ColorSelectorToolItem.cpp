#include "ColorSelectorToolItem.h"

#include <gtk/gtk.h>

#include "control/actions/ActionDatabase.h"
#include "gui/toolbarMenubar/icon/ColorIcon.h"  // for ColorIcon
#include "util/GtkUtil.h"                       // for fixActionableInitialSensitivity
#include "util/i18n.h"                          // for _

ColorSelectorToolItem::ColorSelectorToolItem(ActionDatabase& db):
        AbstractToolItem("COLOR_SELECT", Category::COLORS), gAction(db.getAction(Action::TOOL_COLOR)) {}

auto ColorSelectorToolItem::createItem(ToolbarSide) -> Widgetry {
    GtkWidget* btn = gtk_button_new();
    xoj::util::GVariantSPtr v(g_action_get_state(G_ACTION(gAction.get())), xoj::util::adopt);
    Color initialColor = getGVariantValue<Color>(v.get());
    gtk_button_set_child(GTK_BUTTON(btn), ColorIcon::newGtkImage(initialColor, false));

    gtk_widget_set_tooltip_text(btn, this->getToolDisplayName().c_str());
    gtk_widget_set_sensitive(btn, g_action_get_enabled(G_ACTION(gAction.get())));
    gtk_actionable_set_action_name(GTK_ACTIONABLE(btn),
                                   (std::string("win.") + Action_toString(Action::SELECT_COLOR)).c_str());

    // The color follows the value of the GAction state
    // NB: we use the notify::state signal because "change-state" is not raised by g_simple_action_set_state()
    g_signal_connect_object(gAction.get(), "notify::state", G_CALLBACK(+[](GObject* a, GParamSpec*, gpointer btn) {
                                xoj::util::GVariantSPtr state(g_action_get_state(G_ACTION(a)), xoj::util::adopt);
                                auto c = getGVariantValue<Color>(state.get());
                                gtk_button_set_child(GTK_BUTTON(btn), ColorIcon::newGtkImage(c, false));
                            }),
                            btn, GConnectFlags(0));

    /// Makes a proxy item for the toolbar's overflow menu
    auto createProxy = [&]() {
        GtkWidget* proxy = gtk_button_new();
        auto* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
        gtk_button_set_child(GTK_BUTTON(proxy), box);
        gtk_box_append(GTK_BOX(box), ColorIcon::newGtkImage(Colors::gray, false));
        gtk_box_append(GTK_BOX(box), gtk_label_new(getToolDisplayName().c_str()));
        gtk_actionable_set_action_name(GTK_ACTIONABLE(proxy),
                                       (std::string("win.") + Action_toString(Action::SELECT_COLOR)).c_str());
        gtk_widget_add_css_class(proxy, "model");
        return proxy;
    };
    return {xoj::util::WidgetSPtr(btn, xoj::util::adopt), xoj::util::WidgetSPtr(createProxy(), xoj::util::adopt)};
}

auto ColorSelectorToolItem::getToolDisplayName() const -> std::string { return _("Select color"); }

auto ColorSelectorToolItem::getNewToolIcon() const -> GtkWidget* { return ColorIcon::newGtkImage(Colors::gray, false); }

auto ColorSelectorToolItem::createPaintable(GdkSurface*) const -> xoj::util::GObjectSPtr<GdkPaintable> {
    return ColorIcon::newGdkPaintable(Colors::gray, false);
}
