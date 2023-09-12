#include "ColorSelectorToolItem.h"

#include <utility>  // for move

#include <gtk/gtk.h>

#include "control/actions/ActionDatabase.h"
#include "gui/toolbarMenubar/AbstractToolItem.h"  // for AbstractToolItem
#include "gui/toolbarMenubar/icon/ColorIcon.h"    // for ColorIcon
#include "util/GtkUtil.h"                         // for fixActionableInitialSensitivity
#include "util/gtk4_helper.h"                     // for gtk_button_set_child
#include "util/i18n.h"                            // for _

constexpr int ICON_SIZE = 22;

ColorSelectorToolItem::ColorSelectorToolItem(ActionDatabase& db):
        AbstractToolItem("COLOR_SELECT"), gAction(db.getAction(Action::TOOL_COLOR)) {}

ColorSelectorToolItem::~ColorSelectorToolItem() = default;

static Color GVariant_to_Color(GVariant* v) { return static_cast<Color>(getGVariantValue<uint32_t>(v)); }

auto ColorSelectorToolItem::createItem(bool) -> GtkWidget* {
    GtkWidget* btn = gtk_button_new();
    xoj::util::GVariantSPtr v(g_action_get_state(G_ACTION(gAction.get())), xoj::util::adopt);
    Color initialColor = GVariant_to_Color(v.get());
    gtk_button_set_child(GTK_BUTTON(btn), ColorIcon::newGtkImage(initialColor, ICON_SIZE, false));

    gtk_widget_set_tooltip_text(btn, this->getToolDisplayName().c_str());
    gtk_widget_set_sensitive(btn, g_action_get_enabled(G_ACTION(gAction.get())));
    gtk_actionable_set_action_name(GTK_ACTIONABLE(btn),
                                   (std::string("win.") + Action_toString(Action::SELECT_COLOR)).c_str());

    // The color follows the value of the GAction state
    // NB: we use the notify::state signal because "change-state" is not raised by g_simple_action_set_state()
    g_signal_connect_object(gAction.get(), "notify::state", G_CALLBACK(+[](GObject* a, GParamSpec*, gpointer btn) {
                                xoj::util::GVariantSPtr state(g_action_get_state(G_ACTION(a)), xoj::util::adopt);
                                auto c = GVariant_to_Color(state.get());
                                gtk_button_set_child(GTK_BUTTON(btn), ColorIcon::newGtkImage(c, ICON_SIZE, false));
                            }),
                            btn, GConnectFlags(0));

    this->item.reset(GTK_WIDGET(btn), xoj::util::adopt);
    return this->item.get();
}

auto ColorSelectorToolItem::getToolDisplayName() const -> std::string { return _("Select color"); }

auto ColorSelectorToolItem::getNewToolIcon() const -> GtkWidget* {
    return ColorIcon::newGtkImage(Colors::gray, ICON_SIZE, false);
}
