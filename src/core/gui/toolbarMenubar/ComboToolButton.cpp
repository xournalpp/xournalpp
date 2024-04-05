#include "ComboToolButton.h"

#include <algorithm>  // for find_if
#include <memory>     // for shared_ptr
#include <utility>    // for move

#include <gtk/gtk.h>

#include "util/GtkUtil.h"  // for setToggleButtonUnreleasable
#include "util/glib_casts.h"
#include "util/gtk4_helper.h"  // for gtk_popover_new

/// Returns a floating ref
static GtkWidget* createEmptyButton(GSimpleAction* a, const ComboToolButton::Entry& e) {
    GtkWidget* btn = gtk_toggle_button_new();
    gtk_actionable_set_action_name(GTK_ACTIONABLE(btn), (std::string("win.") + g_action_get_name(G_ACTION(a))).c_str());
    gtk_actionable_set_action_target_value(GTK_ACTIONABLE(btn), e.target.get());
    xoj::util::gtk::setToggleButtonUnreleasable(GTK_TOGGLE_BUTTON(btn));
    gtk_widget_set_tooltip_text(btn, e.name.c_str());
    gtk_widget_set_can_focus(btn, false);  // todo(gtk4) not necessary anymore
    return btn;
}
/// Returns a floating ref
static GtkWidget* createPopoverEntry(GSimpleAction* a, const ComboToolButton::Entry& e) {
    GtkWidget* entry = createEmptyButton(a, e);

    GtkBox* box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6));
    gtk_box_append(box, gtk_image_new_from_icon_name(e.icon.c_str(), GTK_ICON_SIZE_SMALL_TOOLBAR));
    gtk_box_append(box, gtk_label_new(e.name.c_str()));
    gtk_button_set_child(GTK_BUTTON(entry), GTK_WIDGET(box));

    return entry;
}

ComboToolButton::ComboToolButton(std::string id, Category cat, std::string iconName, std::string description,
                                 std::vector<Entry> entries, ActionRef gAction):
        AbstractToolItem(std::move(id), cat),
        entries(std::move(entries)),
        gAction(std::move(gAction)),
        iconName(std::move(iconName)),
        description(std::move(description)) {}

/// Data structure for callbacks
struct ComboToolInstanceData {
    GAction* action;
    GtkButton* btn;
    GtkPopover* popover;
    const std::vector<ComboToolButton::Entry>* entries;
};

/// change the prominent icon depending on G_ACTION(a)'s state
static void setProminentIconCallback(GObject* a, GParamSpec*, ComboToolInstanceData* data) {
    xoj::util::GVariantSPtr state(g_action_get_state(G_ACTION(a)), xoj::util::adopt);
    auto it = std::find_if(data->entries->begin(), data->entries->end(),
                           [s = state.get()](const auto& e) { return g_variant_equal(e.target.get(), s); });
    if (it != data->entries->end()) {
        gtk_button_set_icon_name(data->btn, it->icon.c_str());
        gtk_widget_set_tooltip_text(GTK_WIDGET(data->btn), it->name.c_str());
        gtk_actionable_set_action_target_value(GTK_ACTIONABLE(data->btn), state.get());
        gtk_popover_popdown(data->popover);
    }
};

auto ComboToolButton::createItem(bool horizontal) -> xoj::util::WidgetSPtr {

    auto data = std::make_unique<ComboToolInstanceData>();
    data->entries = &this->entries;
    data->action = G_ACTION(this->gAction.get());

    {  // Create popover
        data->popover = GTK_POPOVER(gtk_popover_new());
        gtk_widget_add_css_class(GTK_WIDGET(data->popover), "toolbar");

        GtkBox* box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
        gtk_popover_set_child(data->popover, GTK_WIDGET(box));

        for (const Entry& t: this->entries) {
            gtk_box_append(box, createPopoverEntry(gAction.get(), t));
        }

        gtk_widget_show_all(GTK_WIDGET(box));
    }

    {                                      // Create prominent button
        const Entry& e = entries.front();  // Select the first entry by default
        data->btn = GTK_BUTTON(createEmptyButton(gAction.get(), e));
        gtk_button_set_icon_name(data->btn, e.icon.c_str());
    }

    // Create item
    GtkMenuButton* menubutton = GTK_MENU_BUTTON(gtk_menu_button_new());
    gtk_widget_set_can_focus(GTK_WIDGET(menubutton), false);  // todo(gtk4) not necessary anymore
    gtk_menu_button_set_popover(menubutton, GTK_WIDGET(data->popover));
    gtk_menu_button_set_direction(menubutton,
                                  horizontal ? GTK_ARROW_DOWN : GTK_ARROW_RIGHT);  // TODO: fix directions

    GtkBox* box = GTK_BOX(gtk_box_new(horizontal ? GTK_ORIENTATION_HORIZONTAL : GTK_ORIENTATION_VERTICAL, 0));
    gtk_box_append(box, GTK_WIDGET(data->btn));
    gtk_box_append(box, GTK_WIDGET(menubutton));

    auto item = xoj::util::WidgetSPtr(GTK_WIDGET(box), xoj::util::adopt);

    // Set up the prominent button according to the action state
    setProminentIconCallback(G_OBJECT(gAction.get()), nullptr, data.get());

    g_signal_connect(gAction.get(), "notify::state", xoj::util::wrap_for_g_callback_v<setProminentIconCallback>,
                     data.get());

    // Disconnect the signal and destroy *data if the widget is destroyed
    g_object_weak_ref(
            G_OBJECT(item.get()),
            +[](gpointer d, GObject* item) {
                auto* data = static_cast<ComboToolInstanceData*>(d);
                g_signal_handlers_disconnect_by_data(data->action, d);
                delete data;
            },
            data.release());

    return item;
}

auto ComboToolButton::getToolDisplayName() const -> std::string { return this->description; }

auto ComboToolButton::getNewToolIcon() const -> GtkWidget* {
    return gtk_image_new_from_icon_name(iconName.c_str(), GTK_ICON_SIZE_SMALL_TOOLBAR);
}
