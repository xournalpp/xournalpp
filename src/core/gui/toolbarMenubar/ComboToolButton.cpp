#include "ComboToolButton.h"

#include <algorithm>  // for find_if
#include <utility>    // for move

#include <gtk/gtk.h>

#include "util/GtkUtil.h"      // for setToggleButtonUnreleasable
#include "util/gtk4_helper.h"  // for gtk_popover_new

namespace {
/// Returns a floating ref
GtkWidget* createEmptyButton(GSimpleAction* a, const ComboToolButton::Entry& e) {
    GtkWidget* btn = gtk_toggle_button_new();
    gtk_actionable_set_action_name(GTK_ACTIONABLE(btn), (std::string("win.") + g_action_get_name(G_ACTION(a))).c_str());
    gtk_actionable_set_action_target_value(GTK_ACTIONABLE(btn), e.target.get());
    xoj::util::gtk::setToggleButtonUnreleasable(GTK_TOGGLE_BUTTON(btn));
    gtk_widget_set_tooltip_text(btn, e.name.c_str());
    gtk_widget_set_can_focus(btn, false);  // todo(gtk4) not necessary anymore
    return btn;
}
/// Returns a floating ref
GtkWidget* createPopoverEntry(GSimpleAction* a, const ComboToolButton::Entry& e) {
    GtkWidget* entry = createEmptyButton(a, e);

    GtkBox* box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6));
    gtk_box_append(box, gtk_image_new_from_icon_name(e.icon.c_str(), GTK_ICON_SIZE_SMALL_TOOLBAR));
    gtk_box_append(box, gtk_label_new(e.name.c_str()));
    gtk_button_set_child(GTK_BUTTON(entry), GTK_WIDGET(box));

    return entry;
}
}  // namespace

ComboToolButton::ComboToolButton(std::string id, std::string iconName, std::string description,
                                 std::vector<Entry> entries, ActionRef gAction):
        AbstractToolItem(std::move(id)),
        entries(std::move(entries)),
        gAction(std::move(gAction)),
        iconName(std::move(iconName)),
        description(std::move(description)) {}

auto ComboToolButton::createItem(bool horizontal) -> GtkWidget* {
    {  // Create popover
        this->popover.reset(gtk_popover_new(), xoj::util::adopt);
        gtk_widget_add_css_class(this->popover.get(), "toolbar");

        GtkBox* box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
        gtk_popover_set_child(GTK_POPOVER(this->popover.get()), GTK_WIDGET(box));

        for (const Entry& t: this->entries) {
            gtk_box_append(box, createPopoverEntry(gAction.get(), t));
        }

        gtk_widget_show_all(GTK_WIDGET(box));
    }

    {                                      // Create prominent button
        const Entry& e = entries.front();  // Select the first entry by default
        this->button.reset(createEmptyButton(gAction.get(), e), xoj::util::adopt);
        GtkWidget* btn = this->button.get();
        gtk_button_set_icon_name(GTK_BUTTON(btn), e.icon.c_str());

        /// change the prominent icon when selecting an entry
        auto setProminentIconCallback = +[](GObject* a, GParamSpec*, ComboToolButton* self) {
            xoj::util::GVariantSPtr state(g_action_get_state(G_ACTION(a)), xoj::util::adopt);
            auto it = std::find_if(self->entries.begin(), self->entries.end(),
                                   [s = state.get()](const Entry& e) { return g_variant_equal(e.target.get(), s); });
            if (it != self->entries.end()) {
                GtkWidget* btn = self->button.get();
                gtk_button_set_icon_name(GTK_BUTTON(btn), it->icon.c_str());
                gtk_widget_set_tooltip_text(btn, it->name.c_str());
                gtk_actionable_set_action_target_value(GTK_ACTIONABLE(btn), state.get());
                gtk_popover_popdown(GTK_POPOVER(self->popover.get()));
            }
        };
        g_signal_connect(gAction.get(), "notify::state", G_CALLBACK(setProminentIconCallback), this);

        // Set up the prominent button according to the action state
        setProminentIconCallback(G_OBJECT(gAction.get()), nullptr, this);
    }

    {  // Create item
        GtkMenuButton* menubutton = GTK_MENU_BUTTON(gtk_menu_button_new());
        gtk_widget_set_can_focus(GTK_WIDGET(menubutton), false);  // todo(gtk4) not necessary anymore
        gtk_menu_button_set_popover(menubutton, popover.get());
        gtk_menu_button_set_direction(menubutton,
                                      horizontal ? GTK_ARROW_DOWN : GTK_ARROW_RIGHT);  // TODO: fix directions

        GtkBox* box = GTK_BOX(gtk_box_new(horizontal ? GTK_ORIENTATION_HORIZONTAL : GTK_ORIENTATION_VERTICAL, 0));
        gtk_box_append(box, this->button.get());
        gtk_box_append(box, GTK_WIDGET(menubutton));

        this->item.reset(GTK_WIDGET(box), xoj::util::adopt);
    }

    return this->item.get();
}

auto ComboToolButton::getToolDisplayName() const -> std::string { return this->description; }

auto ComboToolButton::getNewToolIcon() const -> GtkWidget* {
    return gtk_image_new_from_icon_name(iconName.c_str(), GTK_ICON_SIZE_SMALL_TOOLBAR);
}
