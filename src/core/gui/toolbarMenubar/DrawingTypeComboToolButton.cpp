#include "DrawingTypeComboToolButton.h"

#include <algorithm>  // for find_if
#include <utility>    // for move

#include <gtk/gtk.h>

#include "control/actions/ActionDatabase.h"
#include "gui/IconNameHelper.h"
#include "util/gtk4_helper.h"  // for gtk_popover_new
#include "util/i18n.h"

/// Returns a floating ref
static GtkWidget* createPopoverEntry(const DrawingTypeComboToolButton::Entry& e) {
    GtkWidget* entry = gtk_toggle_button_new();
    gtk_widget_set_can_focus(entry, false);  // todo(gtk4) not necessary anymore
    auto actionName = std::string("win.") + g_action_get_name(G_ACTION(e.gAction.get()));
    gtk_actionable_set_action_name(GTK_ACTIONABLE(entry), actionName.data());
    GtkBox* box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6));
    gtk_button_set_child(GTK_BUTTON(entry), GTK_WIDGET(box));
    gtk_box_append(box, gtk_image_new_from_icon_name(e.icon.c_str(), GTK_ICON_SIZE_SMALL_TOOLBAR));
    gtk_box_append(box, gtk_label_new(e.name.c_str()));
    gtk_widget_set_tooltip_text(entry, e.name.c_str());
    return entry;
}

static EnumIndexedArray<DrawingTypeComboToolButton::Entry, DrawingTypeComboToolButton::Type> makeEntries(
        IconNameHelper& icons, const ActionDatabase& db) {
    using Type = DrawingTypeComboToolButton::Type;
    using Entry = DrawingTypeComboToolButton::Entry;
    EnumIndexedArray<Entry, Type> entries;
    entries[Type::RECTANGLE] = Entry(_("Draw Rectangle"), icons.iconName("draw-rect"), db, Action::TOOL_DRAW_RECTANGLE);
    entries[Type::ELLIPSE] = Entry(_("Draw Ellipse"), icons.iconName("draw-ellipse"), db, Action::TOOL_DRAW_ELLIPSE);
    entries[Type::ARROW] = Entry(_("Draw Arrow"), icons.iconName("draw-arrow"), db, Action::TOOL_DRAW_ARROW);
    entries[Type::DOUBLE_ARROW] =
            Entry(_("Draw Double Arrow"), icons.iconName("draw-double-arrow"), db, Action::TOOL_DRAW_DOUBLE_ARROW);
    entries[Type::LINE] = Entry(_("Draw Line"), icons.iconName("draw-line"), db, Action::TOOL_DRAW_LINE);
    entries[Type::COORDINATE_SYSTEM] = Entry(_("Draw coordinate system"), icons.iconName("draw-coordinate-system"), db,
                                             Action::TOOL_DRAW_COORDINATE_SYSTEM);
    entries[Type::SPLINE] = Entry(_("Draw Spline"), icons.iconName("draw-spline"), db, Action::TOOL_DRAW_SPLINE);
    entries[Type::SHAPE_RECOGNIZER] =
            Entry(_("Stroke recognizer"), icons.iconName("shape-recognizer"), db, Action::TOOL_DRAW_SHAPE_RECOGNIZER);
    return entries;
}

DrawingTypeComboToolButton::Entry::Entry(std::string name, std::string icon, const ActionDatabase& db, Action a):
        name(std::move(name)),
        icon(std::move(icon)),
        gAction(db.getAction(a)),
        fullActionName(std::string("win.") + g_action_get_name(G_ACTION(gAction.get()))) {}

DrawingTypeComboToolButton::DrawingTypeComboToolButton(std::string id, IconNameHelper& icons, const ActionDatabase& db):
        AbstractToolItem(std::move(id)),
        entries(makeEntries(icons, db)),
        iconName(icons.iconName("combo-drawing-type")),
        description(_("Drawing Type Combo")) {}

DrawingTypeComboToolButton::~DrawingTypeComboToolButton() {
    g_signal_handlers_disconnect_by_func(entries[Type::RECTANGLE].gAction.get(),
                                         gpointer(setProminentIconCallback<Type::RECTANGLE>), this);
    g_signal_handlers_disconnect_by_func(entries[Type::ELLIPSE].gAction.get(),
                                         gpointer(setProminentIconCallback<Type::ELLIPSE>), this);
    g_signal_handlers_disconnect_by_func(entries[Type::ARROW].gAction.get(),
                                         gpointer(setProminentIconCallback<Type::ARROW>), this);
    g_signal_handlers_disconnect_by_func(entries[Type::DOUBLE_ARROW].gAction.get(),
                                         gpointer(setProminentIconCallback<Type::DOUBLE_ARROW>), this);
    g_signal_handlers_disconnect_by_func(entries[Type::LINE].gAction.get(),
                                         gpointer(setProminentIconCallback<Type::LINE>), this);
    g_signal_handlers_disconnect_by_func(entries[Type::COORDINATE_SYSTEM].gAction.get(),
                                         gpointer(setProminentIconCallback<Type::COORDINATE_SYSTEM>), this);
    g_signal_handlers_disconnect_by_func(entries[Type::SPLINE].gAction.get(),
                                         gpointer(setProminentIconCallback<Type::SPLINE>), this);
    g_signal_handlers_disconnect_by_func(entries[Type::SHAPE_RECOGNIZER].gAction.get(),
                                         gpointer(setProminentIconCallback<Type::SHAPE_RECOGNIZER>), this);
}

template <DrawingTypeComboToolButton::Type s>
void DrawingTypeComboToolButton::setProminentIconCallback(GObject* action, GParamSpec*,
                                                          DrawingTypeComboToolButton* self) {
    xoj::util::GVariantSPtr state(g_action_get_state(G_ACTION(action)), xoj::util::adopt);
    if (getGVariantValue<bool>(state.get())) {
        auto& e = self->entries[s];
        gtk_button_set_icon_name(GTK_BUTTON(self->button.get()), e.icon.c_str());
        gtk_widget_set_tooltip_text(self->button.get(), e.name.c_str());
        gtk_actionable_set_action_name(GTK_ACTIONABLE(self->button.get()), e.fullActionName.c_str());
    }
    gtk_popover_popdown(GTK_POPOVER(self->popover.get()));
}

auto DrawingTypeComboToolButton::createItem(bool horizontal) -> GtkWidget* {
    {  // Create popover
        this->popover.reset(gtk_popover_new(), xoj::util::adopt);
        GtkBox* box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
        gtk_popover_set_child(GTK_POPOVER(this->popover.get()), GTK_WIDGET(box));
        gtk_widget_add_css_class(this->popover.get(), "toolbar");

        for (const Entry& t: this->entries) {
            gtk_box_append(box, createPopoverEntry(t));
        }

        gtk_widget_show_all(GTK_WIDGET(box));
    }

    {  // Create prominent button
        this->button.reset(gtk_toggle_button_new(), xoj::util::adopt);

        GtkWidget* btn = this->button.get();
        gtk_widget_set_can_focus(btn, false);  // todo(gtk4) not necessary anymore

        auto it = std::find_if(entries.begin(), entries.end(), [](auto& e) {
            xoj::util::GVariantSPtr state(g_action_get_state(G_ACTION(e.gAction.get())), xoj::util::adopt);
            return getGVariantValue<bool>(state.get());
        });
        const Entry& e = it == entries.end() ? entries[Type::RECTANGLE] : *it;  // Select an entry by default
        gtk_actionable_set_action_name(GTK_ACTIONABLE(btn), e.fullActionName.data());
        gtk_button_set_icon_name(GTK_BUTTON(btn), e.icon.c_str());
        gtk_widget_set_tooltip_text(btn, e.name.c_str());
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

    /// change the prominent icon when selecting an entry (by any means)
    g_signal_connect(entries[Type::RECTANGLE].gAction.get(), "notify::state",
                     G_CALLBACK(setProminentIconCallback<Type::RECTANGLE>), this);
    g_signal_connect(entries[Type::ELLIPSE].gAction.get(), "notify::state",
                     G_CALLBACK(setProminentIconCallback<Type::ELLIPSE>), this);
    g_signal_connect(entries[Type::ARROW].gAction.get(), "notify::state",
                     G_CALLBACK(setProminentIconCallback<Type::ARROW>), this);
    g_signal_connect(entries[Type::DOUBLE_ARROW].gAction.get(), "notify::state",
                     G_CALLBACK(setProminentIconCallback<Type::DOUBLE_ARROW>), this);
    g_signal_connect(entries[Type::LINE].gAction.get(), "notify::state",
                     G_CALLBACK(setProminentIconCallback<Type::LINE>), this);
    g_signal_connect(entries[Type::COORDINATE_SYSTEM].gAction.get(), "notify::state",
                     G_CALLBACK(setProminentIconCallback<Type::COORDINATE_SYSTEM>), this);
    g_signal_connect(entries[Type::SPLINE].gAction.get(), "notify::state",
                     G_CALLBACK(setProminentIconCallback<Type::SPLINE>), this);
    g_signal_connect(entries[Type::SHAPE_RECOGNIZER].gAction.get(), "notify::state",
                     G_CALLBACK(setProminentIconCallback<Type::SHAPE_RECOGNIZER>), this);

    return this->item.get();
}

auto DrawingTypeComboToolButton::getToolDisplayName() const -> std::string { return this->description; }

auto DrawingTypeComboToolButton::getNewToolIcon() const -> GtkWidget* {
    return gtk_image_new_from_icon_name(iconName.c_str(), GTK_ICON_SIZE_SMALL_TOOLBAR);
}
